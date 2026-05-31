#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include <pqxx/pqxx>

#include "../../common/config.hpp"
#include "../../common/result.hpp"

namespace inventory {

class ConnectionPool;

class PooledConnection {
public:
    PooledConnection() = default;

    PooledConnection(
        std::shared_ptr<pqxx::connection> conn,
        std::shared_ptr<ConnectionPool> pool);

    ~PooledConnection();

    PooledConnection(const PooledConnection&) = delete;
    PooledConnection& operator=(const PooledConnection&) = delete;

    PooledConnection(PooledConnection&& other) noexcept;
    PooledConnection& operator=(PooledConnection&& other) noexcept;

    [[nodiscard]] pqxx::connection& get();
    [[nodiscard]] explicit operator bool() const noexcept;

private:
    void release();

    std::shared_ptr<pqxx::connection> m_conn;
    std::shared_ptr<ConnectionPool> m_pool;
};

class ConnectionPool : public std::enable_shared_from_this<ConnectionPool> {
public:
    explicit ConnectionPool(const DatabaseConfig& config);

    ConnectionPool(
        std::size_t pool_size,
        std::string host,
        uint16_t port,
        std::string dbname,
        std::string user,
        std::string password,
        std::size_t max_retries = 3,
        std::size_t retry_delay_ms = 1000);

    ~ConnectionPool();

    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    ConnectionPool(ConnectionPool&&) = delete;
    ConnectionPool& operator=(ConnectionPool&&) = delete;

    [[nodiscard]] Result<PooledConnection> acquire();
    void release(std::shared_ptr<pqxx::connection> conn);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] std::size_t available() const noexcept;

private:
    std::shared_ptr<pqxx::connection> create_connection();
    std::string connection_string() const;

    std::size_t m_pool_size;
    std::string m_host;
    uint16_t m_port;
    std::string m_dbname;
    std::string m_user;
    std::string m_password;
    std::size_t m_max_retries;
    std::size_t m_retry_delay_ms;

    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<std::shared_ptr<pqxx::connection>> m_pool;
    std::atomic<std::size_t> m_available{ 0 };
    std::atomic<bool> m_shutdown{ false };
};

} // namespace inventory
