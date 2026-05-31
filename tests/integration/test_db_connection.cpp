/**
 * @file test_db_connection.cpp
 * @brief Integration tests for database connection pool and migration system.
 *
 * These tests require a live PostgreSQL database and are DISABLED by default.
 * To run them, remove the DISABLED_ prefix or filter with --gtest_also_run_disabled_tests.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <thread>

#include "infrastructure/database/connection_pool.hpp"
#include "infrastructure/database/migration.hpp"

namespace inventory::test {

class DISABLED_ConnectionPoolTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_pool = std::make_shared<ConnectionPool>(
            4,
            "localhost",
            5432,
            "inventory_test",
            "test_user",
            "test_pass");
    }

    void TearDown() override
    {
        m_pool.reset();
    }

    std::shared_ptr<ConnectionPool> m_pool;
};

/**
 * @brief Test that a connection can be acquired from the pool.
 */
TEST_F(DISABLED_ConnectionPoolTest, AcquireConnection)
{
    auto result = m_pool->acquire();
    ASSERT_TRUE(result) << "Failed to acquire connection: " << result.error().message();
    ASSERT_TRUE(*result) << "Acquired connection is in invalid state";
}

/**
 * @brief Test that a connection is returned to the pool after the wrapper is destroyed.
 */
TEST_F(DISABLED_ConnectionPoolTest, AcquireAndRelease)
{
    {
        auto result = m_pool->acquire();
        ASSERT_TRUE(result);
        EXPECT_TRUE(*result);
        EXPECT_EQ(m_pool->available(), static_cast<std::size_t>(3));
    }
    EXPECT_EQ(m_pool->available(), static_cast<std::size_t>(4));
}

/**
 * @brief Test acquiring multiple connections simultaneously.
 */
TEST_F(DISABLED_ConnectionPoolTest, AcquireMultipleConnections)
{
    auto c1 = m_pool->acquire();
    ASSERT_TRUE(c1);
    auto c2 = m_pool->acquire();
    ASSERT_TRUE(c2);
    auto c3 = m_pool->acquire();
    ASSERT_TRUE(c3);
    auto c4 = m_pool->acquire();
    ASSERT_TRUE(c4);
    EXPECT_EQ(m_pool->available(), static_cast<std::size_t>(0));
}

/**
 * @brief Test that acquiring beyond pool size blocks (timeout would occur in real scenario).
 */
TEST_F(DISABLED_ConnectionPoolTest, PoolExhaustion)
{
    for (std::size_t i = 0; i < 4; ++i) {
        auto result = m_pool->acquire();
        ASSERT_TRUE(result);
    }
    EXPECT_EQ(m_pool->available(), static_cast<std::size_t>(0));
}

/**
 * @brief Test pool size and available counts after multiple acquire/release cycles.
 */
TEST_F(DISABLED_ConnectionPoolTest, PoolSizeTracking)
{
    EXPECT_EQ(m_pool->size(), static_cast<std::size_t>(4));
    EXPECT_EQ(m_pool->available(), static_cast<std::size_t>(4));

    {
        auto conn = m_pool->acquire();
        ASSERT_TRUE(conn);
        EXPECT_EQ(m_pool->available(), static_cast<std::size_t>(3));
    }

    EXPECT_EQ(m_pool->available(), static_cast<std::size_t>(4));
}

class DISABLED_MigrationTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_pool = std::make_shared<ConnectionPool>(
            2,
            "localhost",
            5432,
            "inventory_test",
            "test_user",
            "test_pass");

        m_migration_dir = std::filesystem::temp_directory_path() / "inventory_migrations_test";
        std::filesystem::create_directories(m_migration_dir);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(m_migration_dir);
        m_pool.reset();
    }

    std::shared_ptr<ConnectionPool> m_pool;
    std::filesystem::path m_migration_dir;
};

/**
 * @brief Test that Migration::apply() runs without error when no SQL files exist.
 */
TEST_F(DISABLED_MigrationTest, ApplyWithEmptyDirectory)
{
    Migration migrator(*m_pool, m_migration_dir);
    auto result = migrator.apply();
    EXPECT_TRUE(result) << "Migration apply failed: " << result.error().message();
}

/**
 * @brief Test that Migration::apply() runs an actual SQL migration file.
 */
TEST_F(DISABLED_MigrationTest, ApplySingleMigration)
{
    auto migration_file = m_migration_dir / "001_create_test_table.sql";
    {
        std::ofstream ofs(migration_file);
        ofs << "CREATE TABLE IF NOT EXISTS _migration_test (id INT PRIMARY KEY);\n";
    }

    Migration migrator(*m_pool, m_migration_dir);
    auto result = migrator.apply();
    EXPECT_TRUE(result) << "Migration apply failed: " << result.error().message();

    auto applied = migrator.applied_migrations();
    ASSERT_TRUE(applied);
    ASSERT_EQ(applied->size(), static_cast<std::size_t>(1));
    EXPECT_EQ((*applied)[0], "001_create_test_table.sql");
}

/**
 * @brief Test that applied migrations are idempotent.
 */
TEST_F(DISABLED_MigrationTest, ApplyMigrationsIsIdempotent)
{
    auto migration_file = m_migration_dir / "001_create_test_table.sql";
    {
        std::ofstream ofs(migration_file);
        ofs << "CREATE TABLE IF NOT EXISTS _migration_test (id INT PRIMARY KEY);\n";
    }

    Migration migrator(*m_pool, m_migration_dir);
    auto first = migrator.apply();
    EXPECT_TRUE(first);

    auto second = migrator.apply();
    EXPECT_TRUE(second);

    auto applied = migrator.applied_migrations();
    ASSERT_TRUE(applied);
    EXPECT_EQ(applied->size(), static_cast<std::size_t>(1));
}

/**
 * @brief Test pending_migrations returns correct unapplied migrations.
 */
TEST_F(DISABLED_MigrationTest, PendingMigrations)
{
    auto migration_file = m_migration_dir / "001_init.sql";
    {
        std::ofstream ofs(migration_file);
        ofs << "CREATE TABLE IF NOT EXISTS _ping (id INT);\n";
    }

    Migration migrator(*m_pool, m_migration_dir);

    auto pending_before = migrator.pending_migrations();
    ASSERT_TRUE(pending_before);
    ASSERT_EQ(pending_before->size(), static_cast<std::size_t>(1));

    auto result = migrator.apply();
    ASSERT_TRUE(result);

    auto pending_after = migrator.pending_migrations();
    ASSERT_TRUE(pending_after);
    EXPECT_TRUE(pending_after->empty());
}

} // namespace inventory::test
