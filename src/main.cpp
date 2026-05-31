#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>

#include <drogon/drogon.h>
#include <spdlog/spdlog.h>
#include <openssl/sha.h>
#include <jwt-cpp/jwt.h>

#include "common/config.hpp"
#include "common/logger.hpp"
#include "core/repositories/item_repository.hpp"
#include "core/repositories/user_repository.hpp"
#include "core/services/auth_service.hpp"
#include "core/use_cases/create_item_use_case.hpp"
#include "core/use_cases/get_item_use_case.hpp"
#include "core/use_cases/list_items_use_case.hpp"
#include "core/use_cases/update_item_use_case.hpp"
#include "core/use_cases/delete_item_use_case.hpp"
#include "core/use_cases/authenticate_user_use_case.hpp"
#include "infrastructure/database/connection_pool.hpp"
#include "infrastructure/database/migration.hpp"
#include "infrastructure/repositories/postgres_item_repository.hpp"
#include "infrastructure/repositories/postgres_user_repository.hpp"
#include "infrastructure/cache/cache_adapter.hpp"
#include "api/controllers/item_controller.hpp"
#include "api/controllers/auth_controller.hpp"
#include "api/middleware/auth_middleware.hpp"
#include "api/middleware/logging_middleware.hpp"

using namespace inventory;

static std::atomic<bool> g_running{ true };

void signal_handler(int signal)
{
    spdlog::warn("Received signal {}, shutting down...", signal);
    g_running = false;
    drogon::app().quit();
}

class AuthServiceImpl final : public AuthService {
public:
    explicit AuthServiceImpl(std::shared_ptr<UserRepository> user_repo, const JwtConfig& jwt_config)
        : m_user_repo(std::move(user_repo))
        , m_jwt_config(jwt_config)
    {
    }

    Result<AuthTokens> authenticate(const std::string& username, const std::string& password) override
    {
        auto user_result = m_user_repo->find_by_username(username);
        if (!user_result) {
            return std::unexpected(user_result.error());
        }

        if (!verify_password(password, user_result.value().password_hash())) {
            return std::unexpected(Error::unauthorized("Invalid credentials"));
        }

        auto access = generate_jwt(user_result.value(), m_jwt_config.access_expiry_minutes * 60);
        auto refresh = generate_jwt(user_result.value(), m_jwt_config.refresh_expiry_days * 86400);

        return AuthTokens{ access, refresh, user_result.value() };
    }

    Result<AuthTokens> refresh_tokens(const std::string& refresh_token) override
    {
        auto payload = decode_jwt(refresh_token);
        if (!payload) {
            return std::unexpected(Error::unauthorized("Invalid refresh token"));
        }

        auto uid = utils::Uuid::from_string(payload.value()["sub"].asString());
        auto user_result = m_user_repo->find_by_id(uid);
        if (!user_result) {
            return std::unexpected(Error::unauthorized("User not found"));
        }

        return authenticate(user_result.value().username(), "");
    }

    Result<void> validate_token(const std::string& token) override
    {
        auto payload = decode_jwt(token);
        if (!payload) {
            return std::unexpected(Error::unauthorized("Invalid token"));
        }
        return {};
    }

    Result<User> get_current_user(const std::string& token) override
    {
        auto payload = decode_jwt(token);
        if (!payload) {
            return std::unexpected(Error::unauthorized("Invalid token"));
        }

        auto uid = utils::Uuid::from_string(payload.value()["sub"].asString());
        return m_user_repo->find_by_id(uid);
    }

    std::string hash_password(const std::string& password) override
    {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(password.data()), password.size(), hash);
        std::string result;
        result.reserve(SHA256_DIGEST_LENGTH * 2);
        for (auto b : hash) {
            result += std::format("{:02x}", b);
        }
        return result;
    }

    bool verify_password(const std::string& password, const std::string& hash) override
    {
        return hash_password(password) == hash;
    }

private:
    std::optional<Json::Value> decode_jwt(const std::string& token)
    {
        try {
            auto decoded = jwt::decode(token);
            Json::Value payload;
            for (auto& claim : decoded.get_payload_claims()) {
                payload[claim.first] = claim.second.to_json();
            }
            return payload;
        } catch (...) {
            return std::nullopt;
        }
    }

    std::string generate_jwt(const User& user, long expiry_seconds)
    {
        auto token = jwt::create()
            .set_issuer(m_jwt_config.issuer)
            .set_subject(user.id().to_string())
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(expiry_seconds))
            .set_payload_claim("role", jwt::claim(std::string(User::role_to_string(user.role()))))
            .set_payload_claim("username", jwt::claim(user.username()))
            .sign(jwt::algorithm::hs256{ m_jwt_config.access_secret });
        return token;
    }

    std::shared_ptr<UserRepository> m_user_repo;
    JwtConfig m_jwt_config;
};

Result<void> initialize_application()
{
    auto& config = Config::instance();

    Logger::init(
        config.log().level,
        config.log().file,
        config.log().max_size_mb,
        config.log().max_files);

    spdlog::info("Starting inventory management system v{}", "1.0.0");

    auto pool = std::make_shared<ConnectionPool>(
        config.database().pool_size,
        config.database().host,
        config.database().port,
        config.database().name,
        config.database().user,
        config.database().password,
        config.database().max_retries,
        config.database().retry_delay_ms);

    spdlog::info("Database connection pool created (size={})", config.database().pool_size);

    Migration migration(*pool, "src/infrastructure/database/migrations");
    auto mig_result = migration.apply();
    if (!mig_result) {
        spdlog::error("Migration failed: {}", mig_result.error().message());
        return std::unexpected(mig_result.error());
    }
    spdlog::info("Database migrations applied successfully");

    auto item_repo = std::make_shared<PostgresItemRepository>(*pool);
    auto user_repo = std::make_shared<PostgresUserRepository>(*pool);
    auto auth_service = std::make_shared<AuthServiceImpl>(user_repo, config.jwt());

    auto create_item_uc = std::make_shared<CreateItemUseCase>(item_repo);
    auto get_item_uc = std::make_shared<GetItemUseCase>(item_repo);
    auto list_items_uc = std::make_shared<ListItemsUseCase>(item_repo);
    auto update_item_uc = std::make_shared<UpdateItemUseCase>(item_repo);
    auto delete_item_uc = std::make_shared<DeleteItemUseCase>(item_repo);
    auto auth_uc = std::make_shared<AuthenticateUserUseCase>(user_repo, auth_service);

    auto item_controller = std::make_shared<ItemController>(
        create_item_uc, get_item_uc, list_items_uc,
        update_item_uc, delete_item_uc);

    auto auth_controller = std::make_shared<AuthController>(auth_uc, auth_service);

    drogon::app().registerController(item_controller);
    drogon::app().registerController(auth_controller);

    auto auth_mw = std::make_shared<AuthMiddleware>(auth_service);
    auto log_mw = std::make_shared<LoggingMiddleware>();
    drogon::app().registerMiddleware(auth_mw);
    drogon::app().registerMiddleware(log_mw);

    drogon::app().addListener(config.server().host, config.server().port);
    drogon::app().setThreadNum(config.server().workers);
    drogon::app().setClientMaxBodySize(10 * 1024 * 1024);
    drogon::app().setIdleConnectionTimeout(config.server().request_timeout_ms);
    drogon::app().enableServerHeader(false);

    spdlog::info("Server configured on {}:{} with {} workers",
        config.server().host, config.server().port, config.server().workers);

    return {};
}

int main(int argc, char* argv[])
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    auto& config = Config::instance();
    auto cfg_result = config.load("config/config.yaml");
    if (!cfg_result) {
        std::cerr << "Failed to load config: " << cfg_result.error().message() << std::endl;
        return 1;
    }

    auto init_result = initialize_application();
    if (!init_result) {
        std::cerr << "Initialization failed: " << init_result.error().message() << std::endl;
        return 1;
    }

    spdlog::info("Application started successfully");
    drogon::app().run();

    spdlog::info("Application shut down gracefully");
    return 0;
}
