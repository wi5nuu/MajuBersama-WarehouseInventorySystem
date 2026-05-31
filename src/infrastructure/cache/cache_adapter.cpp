#include "cache_adapter.hpp"

#include <format>
#include <mutex>

#include <spdlog/spdlog.h>

namespace inventory {

// -----------------------------------------------------------------------------
// InMemoryCacheAdapter
// -----------------------------------------------------------------------------

InMemoryCacheAdapter::InMemoryCacheAdapter(std::chrono::seconds default_ttl)
    : m_default_ttl(default_ttl)
{
}

Result<std::optional<std::string>> InMemoryCacheAdapter::get(const std::string& key)
{
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_entries.find(key);
        if (it == m_entries.end()) {
            return std::optional<std::string>{};
        }

        auto& entry = it->second;

        if (entry.has_ttl && std::chrono::system_clock::now() >= entry.expires_at) {
            m_entries.erase(it);
            return std::optional<std::string>{};
        }

        return std::optional<std::string>(entry.value);
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Cache get error: {}", ex.what())));
    }
}

Result<void> InMemoryCacheAdapter::set(
    const std::string& key,
    const std::string& value,
    std::chrono::seconds ttl)
{
    try {
        auto effective_ttl = ttl;
        if (effective_ttl.count() == 0 && m_default_ttl.count() > 0) {
            effective_ttl = m_default_ttl;
        }

        std::lock_guard<std::mutex> lock(m_mutex);

        Entry entry;
        entry.value = value;

        if (effective_ttl.count() > 0) {
            entry.expires_at = std::chrono::system_clock::now() + effective_ttl;
            entry.has_ttl = true;
        }

        m_entries[key] = std::move(entry);
        return {};
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Cache set error: {}", ex.what())));
    }
}

Result<void> InMemoryCacheAdapter::del(const std::string& key)
{
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.erase(key);
        return {};
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Cache del error: {}", ex.what())));
    }
}

Result<bool> InMemoryCacheAdapter::exists(const std::string& key)
{
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_entries.find(key);
        if (it == m_entries.end()) {
            return false;
        }

        auto& entry = it->second;
        if (entry.has_ttl && std::chrono::system_clock::now() >= entry.expires_at) {
            m_entries.erase(it);
            return false;
        }

        return true;
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Cache exists error: {}", ex.what())));
    }
}

Result<void> InMemoryCacheAdapter::flush()
{
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.clear();
        return {};
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Cache flush error: {}", ex.what())));
    }
}

Result<void> InMemoryCacheAdapter::evict_expired()
{
    try {
        auto now = std::chrono::system_clock::now();
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto it = m_entries.begin(); it != m_entries.end();) {
            if (it->second.has_ttl && now >= it->second.expires_at) {
                it = m_entries.erase(it);
            } else {
                ++it;
            }
        }

        return {};
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(
            std::format("Cache eviction error: {}", ex.what())));
    }
}

} // namespace inventory
