#include "connection_pool.hpp"

#include <format>
#include <thread>

#include <spdlog/spdlog.h>

namespace inventory {

PooledConnection::PooledConnection(
    std::shared_ptr<pqxx::connection> conn,
    std::shared_ptr<ConnectionPool> pool)
    : m_conn(std::move(conn))
    , m_pool(std::move(pool))
{
}

PooledConnection::~PooledConnection()
{
    release();
}

PooledConnection::PooledConnection(PooledConnection&& other) noexcept
    : m_conn(std::move(other.m_conn))
    , m_pool(std::move(other.m_pool))
{
    other.m_conn = nullptr;
}

PooledConnection& PooledConnection::operator=(PooledConnection&& other) noexcept
{
    if (this != &other) {
        release();
        m_conn = std::move(other.m_conn);
        m_pool = std::move(other.m_pool);
        other.m_conn = nullptr;
    }
    return *this;
}

pqxx::connection& PooledConnection::get()
{
    return *m_conn;
}

PooledConnection::operator bool() const noexcept
{
    return m_conn != nullptr && m_conn->is_open();
}

void PooledConnection::release()
{
    if (m_conn && m_pool) {
        if (m_conn->is_open()) {
            m_pool->release(std::move(m_conn));
        }
        m_pool.reset();
    }
}

ConnectionPool::ConnectionPool(const DatabaseConfig& config)
    : m_pool_size(config.pool_size)
    , m_host(config.host)
    , m_port(config.port)
    , m_dbname(config.name)
    , m_user(config.user)
    , m_password(config.password)
    , m_max_retries(config.max_retries)
    , m_retry_delay_ms(config.retry_delay_ms)
{
    for (std::size_t i = 0; i < m_pool_size; ++i) {
        auto conn = create_connection();
        if (conn) {
            m_pool.push(std::move(conn));
            m_available.fetch_add(1);
        }
    }
    spdlog::info("Connection pool initialized with {}/{} connections",
        m_available.load(), m_pool_size);
}

ConnectionPool::ConnectionPool(
    std::size_t pool_size,
    std::string host,
    uint16_t port,
    std::string dbname,
    std::string user,
    std::string password,
    std::size_t max_retries,
    std::size_t retry_delay_ms)
    : m_pool_size(pool_size)
    , m_host(std::move(host))
    , m_port(port)
    , m_dbname(std::move(dbname))
    , m_user(std::move(user))
    , m_password(std::move(password))
    , m_max_retries(max_retries)
    , m_retry_delay_ms(retry_delay_ms)
{
    for (std::size_t i = 0; i < m_pool_size; ++i) {
        auto conn = create_connection();
        if (conn) {
            m_pool.push(std::move(conn));
            m_available.fetch_add(1);
        }
    }
    spdlog::info("Connection pool initialized with {}/{} connections",
        m_available.load(), m_pool_size);
}

ConnectionPool::~ConnectionPool()
{
    m_shutdown.store(true);
    m_cv.notify_all();

    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_pool.empty()) {
        m_pool.pop();
    }
    m_available.store(0);
}

Result<PooledConnection> ConnectionPool::acquire()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_cv.wait(lock, [this] {
        return m_available.load() > 0 || m_shutdown.load();
    });

    if (m_shutdown.load()) {
        return std::unexpected(Error::database("Connection pool is shutting down"));
    }

    auto conn = std::move(m_pool.front());
    m_pool.pop();
    m_available.fetch_sub(1);
    lock.unlock();

    if (!conn->is_open()) {
        spdlog::warn("Reconnecting stale database connection");
        conn = create_connection();
        if (!conn) {
            return std::unexpected(Error::database("Failed to reconnect to database"));
        }
    }

    return PooledConnection(std::move(conn), shared_from_this());
}

void ConnectionPool::release(std::shared_ptr<pqxx::connection> conn)
{
    if (!conn) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pool.push(std::move(conn));
        m_available.fetch_add(1);
    }
    m_cv.notify_one();
}

std::size_t ConnectionPool::size() const noexcept
{
    return m_pool_size;
}

std::size_t ConnectionPool::available() const noexcept
{
    return m_available.load();
}

std::shared_ptr<pqxx::connection> ConnectionPool::create_connection()
{
    auto cs = connection_string();
    std::size_t delay = m_retry_delay_ms;

    for (std::size_t attempt = 0; attempt <= m_max_retries; ++attempt) {
        try {
            auto conn = std::make_shared<pqxx::connection>(cs);
            if (conn->is_open()) {
                return conn;
            }
        } catch (const pqxx::broken_connection& ex) {
            spdlog::warn("Connection attempt {}/{} failed: {}",
                attempt + 1, m_max_retries + 1, ex.what());
        } catch (const std::exception& ex) {
            spdlog::error("Unexpected error creating connection: {}", ex.what());
        }

        if (attempt < m_max_retries) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            delay *= 2;
        }
    }

    spdlog::error("All {} connection attempts failed", m_max_retries + 1);
    return nullptr;
}

std::string ConnectionPool::connection_string() const
{
    return std::format(
        "host={} port={} dbname={} user={} password={}",
        m_host, m_port, m_dbname, m_user, m_password);
}

} // namespace inventory
