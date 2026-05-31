#include "auth_controller.hpp"

#include <drogon/HttpResponse.h>

#include "../dto/auth_dto.hpp"
#include "../dto/error_response.hpp"

namespace inventory {

AuthController::AuthController(
    std::shared_ptr<AuthenticateUserUseCase> auth_use_case,
    std::shared_ptr<AuthService> auth_service)
    : m_auth_use_case(std::move(auth_use_case))
    , m_auth_service(std::move(auth_service))
{
}

drogon::HttpResponsePtr AuthController::make_error_response(
    int status, const std::string& title, const std::string& detail)
{
    ErrorResponse err{
        .type = "about:blank",
        .title = title,
        .status = status,
        .detail = detail,
    };
    auto resp = drogon::HttpResponse::newHttpJsonResponse(err.to_json());
    resp->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    return resp;
}

drogon::HttpResponsePtr AuthController::make_json_response(const nlohmann::json& data)
{
    auto resp = drogon::HttpResponse::newHttpJsonResponse(data);
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    return resp;
}

void AuthController::login(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto json_ptr = req->getJsonObject();
    if (!json_ptr) {
        callback(make_error_response(400, "Bad Request", "Request body must be valid JSON"));
        return;
    }

    nlohmann::json j = nlohmann::json::parse(json_ptr->toStyledString());
    LoginRequest login_req;
    try {
        from_json(j, login_req);
    } catch (const std::exception& e) {
        callback(make_error_response(400, "Bad Request", std::string("Invalid request body: ") + e.what()));
        return;
    }

    if (login_req.username.empty() || login_req.password.empty()) {
        callback(make_error_response(422, "Validation Error", "Username and password are required"));
        return;
    }

    auto result = m_auth_use_case->execute(login_req.username, login_req.password);
    if (!result) {
        auto& err = result.error();
        int status = 401;
        if (err.code() == ErrorCode::kAuthenticationFailed) {
            status = 401;
        } else if (err.code() == ErrorCode::kNotFound) {
            status = 401;
        } else {
            status = 500;
        }
        callback(make_error_response(status, "Authentication Failed", err.message()));
        return;
    }

    auto& tokens = result.value();
    AuthResponse auth_resp{
        .access_token = tokens.access_token,
        .refresh_token = tokens.refresh_token,
        .user = user_to_response(tokens.user),
    };

    callback(make_json_response(auth_resp));
}

void AuthController::refresh(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto json_ptr = req->getJsonObject();
    if (!json_ptr) {
        callback(make_error_response(400, "Bad Request", "Request body must be valid JSON"));
        return;
    }

    nlohmann::json j = nlohmann::json::parse(json_ptr->toStyledString());
    RefreshTokenRequest refresh_req;
    try {
        from_json(j, refresh_req);
    } catch (const std::exception& e) {
        callback(make_error_response(400, "Bad Request", std::string("Invalid request body: ") + e.what()));
        return;
    }

    if (refresh_req.refresh_token.empty()) {
        callback(make_error_response(422, "Validation Error", "Refresh token is required"));
        return;
    }

    auto result = m_auth_service->refresh_tokens(refresh_req.refresh_token);
    if (!result) {
        auto& err = result.error();
        int status = 401;
        if (err.code() == ErrorCode::kTokenExpired) {
            status = 401;
        } else if (err.code() == ErrorCode::kInvalidToken) {
            status = 401;
        } else {
            status = 500;
        }
        callback(make_error_response(status, "Token Refresh Failed", err.message()));
        return;
    }

    auto& tokens = result.value();
    AuthResponse auth_resp{
        .access_token = tokens.access_token,
        .refresh_token = tokens.refresh_token,
        .user = user_to_response(tokens.user),
    };

    callback(make_json_response(auth_resp));
}

void AuthController::logout(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto auth_header = req->getHeader("Authorization");
    if (auth_header.empty() || !auth_header.starts_with("Bearer ")) {
        callback(make_error_response(401, "Unauthorized", "Missing or invalid Authorization header"));
        return;
    }

    auto token = auth_header.substr(7);
    auto result = m_auth_service->validate_token(token);
    if (!result) {
        callback(make_error_response(401, "Unauthorized", "Invalid or expired token"));
        return;
    }

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k204NoContent);
    callback(resp);
}

} // namespace inventory
