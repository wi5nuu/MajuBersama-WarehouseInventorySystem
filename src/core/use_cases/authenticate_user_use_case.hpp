#pragma once

#include <memory>

#include "../entities/user.hpp"
#include "../repositories/user_repository.hpp"
#include "../services/auth_service.hpp"
#include "../../common/result.hpp"

namespace inventory {

class AuthenticateUserUseCase {
public:
    explicit AuthenticateUserUseCase(
        std::shared_ptr<UserRepository> user_repo,
        std::shared_ptr<AuthService> auth_service)
        : m_user_repo(std::move(user_repo))
        , m_auth_service(std::move(auth_service))
    {
    }

    Result<AuthTokens> execute(const std::string& username, const std::string& password);

private:
    std::shared_ptr<UserRepository> m_user_repo;
    std::shared_ptr<AuthService> m_auth_service;
};

} // namespace inventory
