#include "postgres_user_repository.hpp"

#include <chrono>
#include <format>

#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

#include "../../common/utils/time_utils.hpp"
#include "../../core/value_objects/email.hpp"

namespace inventory {

PostgresUserRepository::PostgresUserRepository(ConnectionPool& pool)
    : m_pool(pool)
{
}

Result<User> PostgresUserRepository::find_by_id(const utils::Uuid& id)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "SELECT id, username, email, password_hash, role, is_active, "
            "created_at, updated_at FROM users WHERE id = $1",
            id.to_string());

        if (result.empty()) {
            return std::unexpected(Error::not_found(
                std::format("User not found: {}", id.to_string())));
        }

        txn.commit();
        return row_to_user(result[0]);
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(
            std::format("SQL error in find_by_id: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Error in find_by_id: {}", ex.what())));
    }
}

Result<User> PostgresUserRepository::find_by_username(const std::string& username)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "SELECT id, username, email, password_hash, role, is_active, "
            "created_at, updated_at FROM users WHERE username = $1",
            username);

        if (result.empty()) {
            return std::unexpected(Error::not_found(
                std::format("User not found by username: {}", username)));
        }

        txn.commit();
        return row_to_user(result[0]);
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(
            std::format("SQL error in find_by_username: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Error in find_by_username: {}", ex.what())));
    }
}

Result<User> PostgresUserRepository::find_by_email(const Email& email)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "SELECT id, username, email, password_hash, role, is_active, "
            "created_at, updated_at FROM users WHERE email = $1",
            email.value());

        if (result.empty()) {
            return std::unexpected(Error::not_found(
                std::format("User not found by email: {}", email.value())));
        }

        txn.commit();
        return row_to_user(result[0]);
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(
            std::format("SQL error in find_by_email: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Error in find_by_email: {}", ex.what())));
    }
}

Result<std::vector<User>> PostgresUserRepository::find_all(int page, int page_size)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        auto offset = (page - 1) * page_size;

        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "SELECT id, username, email, password_hash, role, is_active, "
            "created_at, updated_at FROM users ORDER BY created_at DESC "
            "LIMIT $1 OFFSET $2",
            page_size, offset);

        std::vector<User> users;
        users.reserve(result.size());
        for (const auto& row : result) {
            users.emplace_back(row_to_user(row));
        }

        txn.commit();
        return users;
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(
            std::format("SQL error in find_all: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Error in find_all: {}", ex.what())));
    }
}

Result<User> PostgresUserRepository::save(const User& user)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "INSERT INTO users (id, username, email, password_hash, role, is_active, "
            "created_at, updated_at) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7, $8) "
            "RETURNING id, username, email, password_hash, role, is_active, "
            "created_at, updated_at",
            user.id().to_string(),
            user.username(),
            user.email().value(),
            user.password_hash(),
            std::string(User::role_to_string(user.role())),
            user.is_active(),
            utils::format_iso8601(user.created_at()),
            utils::format_iso8601(user.updated_at()));

        if (result.empty()) {
            return std::unexpected(Error::database("Failed to insert user"));
        }

        txn.commit();
        return row_to_user(result[0]);
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(
            std::format("SQL error in save: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Error in save: {}", ex.what())));
    }
}

Result<User> PostgresUserRepository::update(const User& user)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "UPDATE users SET "
            "username = $2, email = $3, password_hash = $4, role = $5, "
            "is_active = $6, updated_at = $7 "
            "WHERE id = $1 "
            "RETURNING id, username, email, password_hash, role, is_active, "
            "created_at, updated_at",
            user.id().to_string(),
            user.username(),
            user.email().value(),
            user.password_hash(),
            std::string(User::role_to_string(user.role())),
            user.is_active(),
            utils::format_iso8601(user.updated_at()));

        if (result.empty()) {
            return std::unexpected(Error::not_found(
                std::format("User not found for update: {}", user.id().to_string())));
        }

        txn.commit();
        return row_to_user(result[0]);
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(
            std::format("SQL error in update: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Error in update: {}", ex.what())));
    }
}

Result<void> PostgresUserRepository::remove(const utils::Uuid& id)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "DELETE FROM users WHERE id = $1",
            id.to_string());

        if (result.affected_rows() == 0) {
            return std::unexpected(Error::not_found(
                std::format("User not found for deletion: {}", id.to_string())));
        }

        txn.commit();
        return {};
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(
            std::format("SQL error in remove: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Error in remove: {}", ex.what())));
    }
}

User PostgresUserRepository::row_to_user(const pqxx::row& row)
{
    auto email_result = Email::create(row["email"].as<std::string>());
    auto email = email_result.value_or(Email::create("unknown@unknown.com").value());

    return User(
        utils::Uuid::from_string(row["id"].as<std::string>()),
        row["username"].as<std::string>(),
        email,
        row["password_hash"].as<std::string>(),
        User::string_to_role(row["role"].as<std::string>("viewer")),
        row["is_active"].as<bool>(true),
        utils::parse_iso8601(row["created_at"].as<std::string>()),
        utils::parse_iso8601(row["updated_at"].as<std::string>()));
}

} // namespace inventory
