#include "authenticate_user_use_case.hpp"

namespace inventory {

Result<AuthTokens> AuthenticateUserUseCase::execute(const std::string& username, const std::string& password)
{
    if (username.empty() || password.empty()) {
        return std::unexpected(Error::validation("Username and password are required"));
    }

    auto user_result = m_user_repo->find_by_username(username);
    if (!user_result) {
        return std::unexpected(Error::unauthorized("Invalid username or password"));
    }

    const auto& user = user_result.value();
    if (!user.is_active()) {
        return std::unexpected(Error::forbidden("Account is deactivated"));
    }

    if (!m_auth_service->verify_password(password, user.password_hash())) {
        return std::unexpected(Error::unauthorized("Invalid username or password"));
    }

    return m_auth_service->authenticate(username, password);
}

} // namespace inventory
