#pragma once

#include <string>

#include "../../common/result.hpp"
#include "../entities/user.hpp"

namespace inventory {

struct AuthTokens {
    std::string access_token;
    std::string refresh_token;
    User user;
};

class AuthService {
public:
    virtual ~AuthService() = default;

    virtual Result<AuthTokens> authenticate(const std::string& username, const std::string& password) = 0;
    virtual Result<AuthTokens> refresh_tokens(const std::string& refresh_token) = 0;
    virtual Result<void> validate_token(const std::string& token) = 0;
    virtual Result<User> get_current_user(const std::string& token) = 0;
    virtual std::string hash_password(const std::string& password) = 0;
    virtual bool verify_password(const std::string& password, const std::string& hash) = 0;
};

} // namespace inventory
