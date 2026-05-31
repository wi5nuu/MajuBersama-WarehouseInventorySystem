#pragma once

#include <drogon/HttpController.h>
#include <nlohmann/json.hpp>
#include <memory>

#include "../../core/services/auth_service.hpp"
#include "../../core/use_cases/authenticate_user_use_case.hpp"

namespace inventory {

/**
 * @brief HTTP controller for authentication operations
 * Path prefix: /api/v1/auth
 */
class AuthController : public drogon::HttpController<AuthController> {
public:
    explicit AuthController(std::shared_ptr<AuthenticateUserUseCase> auth_use_case,
        std::shared_ptr<AuthService> auth_service);

    METHOD_LIST_BEGIN
    METHOD_ADD(AuthController::login, "/login", drogon::Post);
    METHOD_ADD(AuthController::refresh, "/refresh", drogon::Post);
    METHOD_ADD(AuthController::logout, "/logout", drogon::Post);
    METHOD_LIST_END

    /**
     * @brief POST /api/v1/auth/login — authenticate user and return tokens
     */
    void login(const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /**
     * @brief POST /api/v1/auth/refresh — refresh access token
     */
    void refresh(const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /**
     * @brief POST /api/v1/auth/logout — invalidate refresh token
     */
    void logout(const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

private:
    std::shared_ptr<AuthenticateUserUseCase> m_auth_use_case;
    std::shared_ptr<AuthService> m_auth_service;

    static drogon::HttpResponsePtr make_error_response(int status, const std::string& title, const std::string& detail);
    static drogon::HttpResponsePtr make_json_response(const nlohmann::json& data);
};

} // namespace inventory
