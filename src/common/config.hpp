#pragma once

#include <filesystem>
#include <format>
#include <string>
#include <unordered_map>

#include <yaml-cpp/yaml.h>

#include "result.hpp"

namespace inventory {

struct DatabaseConfig {
    std::string host = "localhost";
    uint16_t port = 5432;
    std::string name = "inventory_db";
    std::string user = "inventory_user";
    std::string password;
    std::size_t pool_size = 10;
    std::size_t max_retries = 3;
    std::size_t retry_delay_ms = 1000;
    std::string ssl_mode = "prefer";
};

struct ServerConfig {
    std::string host = "0.0.0.0";
    uint16_t port = 8080;
    std::size_t workers = 4;
    std::size_t request_timeout_ms = 30000;
};

struct JwtConfig {
    std::string access_secret;
    std::string refresh_secret;
    std::size_t access_expiry_minutes = 15;
    std::size_t refresh_expiry_days = 7;
    std::string issuer = "pt-maju-bersama";
};

struct LogConfig {
    std::string level = "info";
    std::string file = "/var/log/inventory/app.log";
    std::size_t max_size_mb = 100;
    std::size_t max_files = 7;
    std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v";
};

struct CacheConfig {
    std::string host = "localhost";
    uint16_t port = 6379;
    std::size_t db = 0;
    std::size_t ttl_seconds = 300;
};

class Config {
public:
    static Config& instance()
    {
        static Config cfg;
        return cfg;
    }

    Result<void> load(const std::filesystem::path& path)
    {
        try {
            if (!std::filesystem::exists(path)) {
                return Error::configuration(
                    std::format("Config file not found: {}", path.string()));
            }

            YAML::Node node = YAML::LoadFile(path.string());

            if (node["server"]) {
                auto& srv = node["server"];
                m_server.host = srv["host"].as<std::string>(m_server.host);
                m_server.port = srv["port"].as<uint16_t>(m_server.port);
                m_server.workers = srv["workers"].as<std::size_t>(m_server.workers);
                m_server.request_timeout_ms = srv["request_timeout_ms"].as<std::size_t>(m_server.request_timeout_ms);
            }

            if (node["database"]) {
                auto& db = node["database"];
                m_database.host = db["host"].as<std::string>(m_database.host);
                m_database.port = db["port"].as<uint16_t>(m_database.port);
                m_database.name = db["name"].as<std::string>(m_database.name);
                m_database.user = db["user"].as<std::string>(m_database.user);
                m_database.password = resolve_env(db["password"].as<std::string>(""));
                m_database.pool_size = db["pool_size"].as<std::size_t>(m_database.pool_size);
                m_database.max_retries = db["max_retries"].as<std::size_t>(m_database.max_retries);
                m_database.retry_delay_ms = db["retry_delay_ms"].as<std::size_t>(m_database.retry_delay_ms);
                m_database.ssl_mode = db["ssl_mode"].as<std::string>(m_database.ssl_mode);
            }

            if (node["jwt"]) {
                auto& jwt = node["jwt"];
                m_jwt.access_secret = resolve_env(jwt["access_secret"].as<std::string>(""));
                m_jwt.refresh_secret = resolve_env(jwt["refresh_secret"].as<std::string>(""));
                m_jwt.access_expiry_minutes = jwt["access_expiry_minutes"].as<std::size_t>(m_jwt.access_expiry_minutes);
                m_jwt.refresh_expiry_days = jwt["refresh_expiry_days"].as<std::size_t>(m_jwt.refresh_expiry_days);
                m_jwt.issuer = jwt["issuer"].as<std::string>(m_jwt.issuer);
            }

            if (node["logging"]) {
                auto& log = node["logging"];
                m_log.level = log["level"].as<std::string>(m_log.level);
                m_log.file = log["file"].as<std::string>(m_log.file);
                m_log.max_size_mb = log["max_size_mb"].as<std::size_t>(m_log.max_size_mb);
                m_log.max_files = log["max_files"].as<std::size_t>(m_log.max_files);
                m_log.pattern = log["pattern"].as<std::string>(m_log.pattern);
            }

            if (node["cache"]) {
                auto& cache = node["cache"];
                m_cache.host = cache["host"].as<std::string>(m_cache.host);
                m_cache.port = cache["port"].as<uint16_t>(m_cache.port);
                m_cache.db = cache["db"].as<std::size_t>(m_cache.db);
                m_cache.ttl_seconds = cache["ttl_seconds"].as<std::size_t>(m_cache.ttl_seconds);
            }

            return {};
        } catch (const YAML::Exception& ex) {
            return Error::configuration(std::format("YAML parse error: {}", ex.what()));
        } catch (const std::exception& ex) {
            return Error::configuration(std::format("Config load error: {}", ex.what()));
        }
    }

    [[nodiscard]] const ServerConfig& server() const noexcept { return m_server; }
    [[nodiscard]] const DatabaseConfig& database() const noexcept { return m_database; }
    [[nodiscard]] const JwtConfig& jwt() const noexcept { return m_jwt; }
    [[nodiscard]] const LogConfig& log() const noexcept { return m_log; }
    [[nodiscard]] const CacheConfig& cache() const noexcept { return m_cache; }

private:
    Config() = default;

    static std::string resolve_env(const std::string& val)
    {
        if (val.size() > 2 && val.front() == '$' && val[1] == '{' && val.back() == '}') {
            auto env_var = val.substr(2, val.size() - 3);
            auto* env_val = std::getenv(env_var.c_str());
            return env_val ? std::string(env_val) : "";
        }
        return val;
    }

    ServerConfig m_server;
    DatabaseConfig m_database;
    JwtConfig m_jwt;
    LogConfig m_log;
    CacheConfig m_cache;
};

} // namespace inventory
