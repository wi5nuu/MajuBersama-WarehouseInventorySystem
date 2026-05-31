#include "migration.hpp"
#include "connection_pool.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <sstream>

#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

namespace inventory {

Migration::Migration(ConnectionPool& pool, std::filesystem::path dir)
    : m_pool(pool)
    , m_directory(std::move(dir))
{
}

Result<void> Migration::apply()
{
    auto ensure = ensure_migrations_table();
    if (!ensure) {
        return std::unexpected(Error::database(
            std::format("Failed to ensure migrations table: {}", ensure.error().message())));
    }

    auto files = migration_files();
    if (!files) {
        return files.error();
    }

    for (const auto& filename : *files) {
        auto applied = is_applied(filename);
        if (!applied) {
            return std::unexpected(Error::database(
                std::format("Failed to check migration status: {}", applied.error().message())));
        }

        if (*applied) {
            spdlog::debug("Migration already applied: {}", filename);
            continue;
        }

        auto result = apply_migration(filename);
        if (!result) {
            return std::unexpected(Error::database(
                std::format("Failed to apply migration {}: {}", filename, result.error().message())));
        }

        spdlog::info("Applied migration: {}", filename);
    }

    return {};
}

Result<std::vector<std::string>> Migration::applied_migrations()
{
    auto ensure = ensure_migrations_table();
    if (!ensure) {
        return std::unexpected(Error::database(
            std::format("Failed to ensure migrations table: {}", ensure.error().message())));
    }

    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec("SELECT filename FROM schema_migrations ORDER BY filename ASC");

        std::vector<std::string> migrations;
        migrations.reserve(result.size());
        for (const auto& row : result) {
            migrations.emplace_back(row["filename"].as<std::string>());
        }

        txn.commit();
        return migrations;
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(std::format("SQL error querying migrations: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::database(std::format("Error querying migrations: {}", ex.what())));
    }
}

Result<std::vector<std::string>> Migration::pending_migrations()
{
    auto applied = applied_migrations();
    if (!applied) {
        return applied.error();
    }

    auto files = migration_files();
    if (!files) {
        return files.error();
    }

    std::vector<std::string> pending;
    std::set_difference(
        files->begin(), files->end(),
        applied->begin(), applied->end(),
        std::back_inserter(pending));

    return pending;
}

Result<void> Migration::ensure_migrations_table()
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        txn.exec(R"(
            CREATE TABLE IF NOT EXISTS schema_migrations (
                filename    VARCHAR(255) PRIMARY KEY,
                applied_at  TIMESTAMPTZ NOT NULL DEFAULT NOW()
            )
        )");
        txn.commit();
        return {};
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(
            std::format("Failed to create schema_migrations table: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::database(
            std::format("Unexpected error creating migrations table: {}", ex.what())));
    }
}

Result<std::vector<std::string>> Migration::migration_files()
{
    if (!std::filesystem::exists(m_directory) || !std::filesystem::is_directory(m_directory)) {
        return std::unexpected(Error::configuration(
            std::format("Migration directory does not exist: {}", m_directory.string())));
    }

    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(m_directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".sql") {
            files.emplace_back(entry.path().filename().string());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

Result<void> Migration::apply_migration(const std::string& filename)
{
    auto path = m_directory / filename;

    std::ifstream stream(path);
    if (!stream.is_open()) {
        return std::unexpected(Error::database(
            std::format("Cannot open migration file: {}", path.string())));
    }

    std::stringstream buffer;
    buffer << stream.rdbuf();
    auto sql = buffer.str();

    if (sql.empty()) {
        return std::unexpected(Error::database(
            std::format("Migration file is empty: {}", filename)));
    }

    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        txn.exec(sql);
        txn.exec_params(
            "INSERT INTO schema_migrations (filename) VALUES ($1)",
            filename);
        txn.commit();
        return {};
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(std::format(
            "Migration {} failed: {}", filename, ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::database(std::format(
            "Migration {} error: {}", filename, ex.what())));
    }
}

Result<bool> Migration::is_applied(const std::string& filename)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "SELECT COUNT(*) AS cnt FROM schema_migrations WHERE filename = $1",
            filename);

        auto count = result[0]["cnt"].as<std::size_t>();
        txn.commit();
        return count > 0;
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(
            std::format("SQL error checking migration: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::database(
            std::format("Error checking migration: {}", ex.what())));
    }
}

Result<void> Migration::record_migration(const std::string& filename)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        txn.exec_params(
            "INSERT INTO schema_migrations (filename) VALUES ($1) ON CONFLICT DO NOTHING",
            filename);
        txn.commit();
        return {};
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(
            std::format("Failed to record migration: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::database(
            std::format("Error recording migration: {}", ex.what())));
    }
}

} // namespace inventory
