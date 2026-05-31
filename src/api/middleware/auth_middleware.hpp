#pragma once

#include <drogon/HttpMiddleware.h>
#include <memory>

#include "../../core/services/auth_service.hpp"

namespace inventory {

/**
 * @brief Middleware that validates JWT Bearer tokens on protected routes
 *
 * Extracts the token from the Authorization header, validates it via AuthService,
 * and sets user_id and user_role attributes on the request for downstream handlers.
 */
class AuthMiddleware : public drogon::HttpMiddleware<AuthMiddleware> {
public:
    explicit AuthMiddleware(std::shared_ptr<AuthService> auth_service);

    void invoke(const drogon::HttpRequestPtr& req,
        drogon::MiddlewareNextCallback&& nextCb,
        drogon::MiddlewareCallback&& mcb) override;

private:
    std::shared_ptr<AuthService> m_auth_service;
};

} // namespace inventory
