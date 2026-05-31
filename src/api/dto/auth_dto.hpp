#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "../../common/utils/uuid.hpp"
#include "../../core/entities/user.hpp"

namespace inventory {

struct LoginRequest {
    std::string username;
    std::string password;
};

struct RefreshTokenRequest {
    std::string refresh_token;
};

struct UserResponse {
    std::string id;
    std::string username;
    std::string email;
    std::string role;
};

struct AuthResponse {
    std::string access_token;
    std::string refresh_token;
    UserResponse user;
};

inline void to_json(nlohmann::json& j, const LoginRequest& req)
{
    j = nlohmann::json{
        {"username", req.username},
        {"password", req.password},
    };
}

inline void from_json(const nlohmann::json& j, LoginRequest& req)
{
    j.at("username").get_to(req.username);
    j.at("password").get_to(req.password);
}

inline void to_json(nlohmann::json& j, const RefreshTokenRequest& req)
{
    j = nlohmann::json{
        {"refresh_token", req.refresh_token},
    };
}

inline void from_json(const nlohmann::json& j, RefreshTokenRequest& req)
{
    j.at("refresh_token").get_to(req.refresh_token);
}

inline void to_json(nlohmann::json& j, const UserResponse& resp)
{
    j = nlohmann::json{
        {"id", resp.id},
        {"username", resp.username},
        {"email", resp.email},
        {"role", resp.role},
    };
}

inline void from_json(const nlohmann::json& j, UserResponse& resp)
{
    j.at("id").get_to(resp.id);
    j.at("username").get_to(resp.username);
    j.at("email").get_to(resp.email);
    j.at("role").get_to(resp.role);
}

inline void to_json(nlohmann::json& j, const AuthResponse& resp)
{
    j = nlohmann::json{
        {"access_token", resp.access_token},
        {"refresh_token", resp.refresh_token},
        {"user", resp.user},
    };
}

inline UserResponse user_to_response(const User& user)
{
    return UserResponse{
        user.id().to_string(),
        user.username(),
        user.email().value(),
        std::string(User::role_to_string(user.role())),
    };
}

} // namespace inventory
