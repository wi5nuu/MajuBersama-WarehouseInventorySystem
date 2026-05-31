#include "auth_middleware.hpp"

#include <drogon/HttpResponse.h>

#include "../dto/error_response.hpp"

namespace inventory {

AuthMiddleware::AuthMiddleware(std::shared_ptr<AuthService> auth_service)
    : m_auth_service(std::move(auth_service))
{
}

void AuthMiddleware::invoke(
    const drogon::HttpRequestPtr& req,
    drogon::MiddlewareNextCallback&& nextCb,
    drogon::MiddlewareCallback&& mcb)
{
    auto auth_header = req->getHeader("Authorization");
    if (auth_header.empty()) {
        ErrorResponse err{
            .type = "about:blank",
            .title = "Unauthorized",
            .status = 401,
            .detail = "Missing Authorization header",
        };
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err.to_json());
        resp->setStatusCode(drogon::k401Unauthorized);
        mcb(resp);
        return;
    }

    if (!auth_header.starts_with("Bearer ")) {
        ErrorResponse err{
            .type = "about:blank",
            .title = "Unauthorized",
            .status = 401,
            .detail = "Authorization header must use Bearer scheme",
        };
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err.to_json());
        resp->setStatusCode(drogon::k401Unauthorized);
        mcb(resp);
        return;
    }

    auto token = auth_header.substr(7);
    auto validation_result = m_auth_service->validate_token(token);
    if (!validation_result) {
        ErrorResponse err{
            .type = "about:blank",
            .title = "Unauthorized",
            .status = 401,
            .detail = validation_result.error().message(),
        };
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err.to_json());
        resp->setStatusCode(drogon::k401Unauthorized);
        mcb(resp);
        return;
    }

    auto user_result = m_auth_service->get_current_user(token);
    if (user_result) {
        req->getAttributes()->insert("user_id", user_result.value().id().to_string());
        req->getAttributes()->insert("user_role",
            std::string(User::role_to_string(user_result.value().role())));
    }

    nextCb(std::move(mcb));
}

} // namespace inventory
