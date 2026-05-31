#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../../common/result.hpp"

namespace inventory {

class ConnectionPool;

/**
 * @brief Database migration system that reads SQL files and applies them in order.
 *
 * Tracks applied migrations in a schema_migrations table and only applies
 * pending migrations that have not yet been recorded.
 */
class Migration {
public:
    /**
     * @brief Construct the migrator with a connection pool and migration directory.
     *
     * @param pool  The connection pool to use.
     * @param dir   Path to the directory containing numbered SQL migration files.
     */
    explicit Migration(ConnectionPool& pool, std::filesystem::path dir);

    /**
     * @brief Apply all pending migrations.
     *
     * @return Result<void> Ok on success, Error on failure.
     */
    Result<void> apply();

    /**
     * @brief Get the list of applied migration filenames.
     *
     * @return Result<std::vector<std::string>> Sorted list of applied migration names.
     */
    Result<std::vector<std::string>> applied_migrations();

    /**
     * @brief Get the list of pending (not yet applied) migration filenames.
     *
     * @return Result<std::vector<std::string>> Sorted list of pending migration names.
     */
    Result<std::vector<std::string>> pending_migrations();

private:
    Result<void> ensure_migrations_table();
    Result<std::vector<std::string>> migration_files();
    Result<void> apply_migration(const std::string& filename);
    Result<bool> is_applied(const std::string& filename);
    Result<void> record_migration(const std::string& filename);

    ConnectionPool& m_pool;
    std::filesystem::path m_directory;
};

} // namespace inventory
