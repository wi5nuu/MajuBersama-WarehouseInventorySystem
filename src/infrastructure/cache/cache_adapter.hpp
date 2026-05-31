#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

#include "../../common/result.hpp"

namespace inventory {

/**
 * @brief Abstract interface for cache operations with TTL support.
 *
 * Provides get, set, del, and exists operations with optional
 * time-to-live expiration. Implementations may back onto Redis,
 * memcached, or an in-memory store.
 */
class CacheAdapter {
public:
    virtual ~CacheAdapter() = default;

    /**
     * @brief Retrieve a value by key.
     *
     * @param key The cache key.
     * @return Result<std::optional<std::string>> The value if found, std::nullopt if not present.
     */
    virtual Result<std::optional<std::string>> get(const std::string& key) = 0;

    /**
     * @brief Store a value with an optional TTL.
     *
     * @param key   The cache key.
     * @param value The value to store.
     * @param ttl   Time-to-live duration (0 means no expiration).
     * @return Result<void> Ok on success.
     */
    virtual Result<void> set(const std::string& key,
        const std::string& value,
        std::chrono::seconds ttl = std::chrono::seconds(0)) = 0;

    /**
     * @brief Delete a key from the cache.
     *
     * @param key The cache key to remove.
     * @return Result<void> Ok on success.
     */
    virtual Result<void> del(const std::string& key) = 0;

    /**
     * @brief Check if a key exists in the cache.
     *
     * @param key The cache key.
     * @return Result<bool> True if the key exists and has not expired.
     */
    virtual Result<bool> exists(const std::string& key) = 0;

    /**
     * @brief Clear all entries from the cache.
     *
     * @return Result<void> Ok on success.
     */
    virtual Result<void> flush() = 0;
};

/**
 * @brief Simple thread-safe in-memory implementation of CacheAdapter.
 *
 * Intended for development and testing when an external cache (e.g. Redis)
 * is not available. Entries are stored with optional TTL and are lazily
 * evicted on access.
 */
class InMemoryCacheAdapter : public CacheAdapter {
public:
    InMemoryCacheAdapter() = default;

    /**
     * @brief Construct the cache with a default TTL.
     *
     * @param default_ttl Default time-to-live applied when no TTL is specified at set().
     */
    explicit InMemoryCacheAdapter(std::chrono::seconds default_ttl);

    Result<std::optional<std::string>> get(const std::string& key) override;

    Result<void> set(const std::string& key,
        const std::string& value,
        std::chrono::seconds ttl = std::chrono::seconds(0)) override;

    Result<void> del(const std::string& key) override;

    Result<bool> exists(const std::string& key) override;

    Result<void> flush() override;

    /**
     * @brief Remove all expired entries from the cache.
     */
    Result<void> evict_expired();

private:
    struct Entry {
        std::string value;
        std::chrono::system_clock::time_point expires_at;
        bool has_ttl{ false };
    };

    std::chrono::seconds m_default_ttl{ 0 };
    std::mutex m_mutex;
    std::unordered_map<std::string, Entry> m_entries;
};

} // namespace inventory
