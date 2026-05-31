#include <gtest/gtest.h>
#include <memory>
#include "core/use_cases/authenticate_user_use_case.hpp"

namespace inventory::test {

class MockAuthUserRepository : public UserRepository {
public:
    std::unordered_map<std::string, User> users_by_username;

    Result<User> find_by_id(const utils::Uuid&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<User> find_by_username(const std::string& username) override
    {
        auto it = users_by_username.find(username);
        if (it != users_by_username.end()) {
            return it->second;
        }
        return std::unexpected(Error::not_found("User not found"));
    }

    Result<User> find_by_email(const Email&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<std::vector<User>> find_all(int, int) override
    {
        return std::vector<User>{};
    }

    Result<User> save(const User&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<User> update(const User&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<void> remove(const utils::Uuid&) override
    {
        return std::unexpected(Error::not_found());
    }

    void add_user(const User& user)
    {
        users_by_username[user.username()] = user;
    }
};

class MockAuthService : public AuthService {
public:
    bool verify_password_result{ true };
    Result<AuthTokens> authenticate_result;

    Result<AuthTokens> authenticate(const std::string& username, const std::string&) override
    {
        if (authenticate_result.has_value()) {
            return authenticate_result.value();
        }
        AuthTokens tokens;
        tokens.access_token = "access_token_" + username;
        tokens.refresh_token = "refresh_token_" + username;
        return tokens;
    }

    Result<AuthTokens> refresh_tokens(const std::string&) override
    {
        return std::unexpected(Error::unauthorized());
    }

    Result<void> validate_token(const std::string&) override
    {
        return {};
    }

    Result<User> get_current_user(const std::string&) override
    {
        return std::unexpected(Error::unauthorized());
    }

    std::string hash_password(const std::string& pwd) override
    {
        return "hashed_" + pwd;
    }

    bool verify_password(const std::string&, const std::string&) override
    {
        return verify_password_result;
    }
};

class TestAuthenticateUserUseCase : public ::testing::Test {
protected:
    std::shared_ptr<MockAuthUserRepository> user_repo;
    std::shared_ptr<MockAuthService> auth_service;
    std::unique_ptr<AuthenticateUserUseCase> use_case;

    void SetUp() override
    {
        user_repo = std::make_shared<MockAuthUserRepository>();
        auth_service = std::make_shared<MockAuthService>();
        use_case = std::make_unique<AuthenticateUserUseCase>(user_repo, auth_service);

        auto now = std::chrono::system_clock::now();
        auto email = Email::create("user@example.com").value();
        User user(utils::Uuid::generate(), "john", email, "hashed_pass",
            UserRole::kOperator, true, now, now);
        user_repo->add_user(user);
    }
};

TEST_F(TestAuthenticateUserUseCase, SuccessfulAuthentication)
{
    auto result = use_case->execute("john", "correct_password");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("access_token_john", result->access_token);
    EXPECT_EQ("refresh_token_john", result->refresh_token);
}

TEST_F(TestAuthenticateUserUseCase, AuthenticationReturnsUser)
{
    auto result = use_case->execute("john", "correct_password");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("john", result->user.username());
    EXPECT_TRUE(result->user.is_active());
}

TEST_F(TestAuthenticateUserUseCase, EmptyUsernameFails)
{
    auto result = use_case->execute("", "password");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestAuthenticateUserUseCase, EmptyPasswordFails)
{
    auto result = use_case->execute("john", "");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestAuthenticateUserUseCase, BothEmptyFails)
{
    auto result = use_case->execute("", "");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestAuthenticateUserUseCase, NonExistentUsernameFails)
{
    auto result = use_case->execute("unknown", "password");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kUnauthorized, result.error().code());
}

TEST_F(TestAuthenticateUserUseCase, WrongPasswordFails)
{
    auth_service->verify_password_result = false;
    auto result = use_case->execute("john", "wrong_password");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kUnauthorized, result.error().code());
}

TEST_F(TestAuthenticateUserUseCase, DeactivatedUserFails)
{
    auto now = std::chrono::system_clock::now();
    auto email = Email::create("deactivated@example.com").value();
    User deactivated(utils::Uuid::generate(), "deactivated_user", email, "hash",
        UserRole::kViewer, false, now, now);
    user_repo->add_user(deactivated);

    auto result = use_case->execute("deactivated_user", "password");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kForbidden, result.error().code());
}

TEST_F(TestAuthenticateUserUseCase, AuthServiceAuthenticateCalledOnSuccess)
{
    bool called = false;
    auth_service->authenticate_result = [&]() -> Result<AuthTokens> {
        called = true;
        AuthTokens tokens;
        tokens.access_token = "custom_token";
        tokens.refresh_token = "custom_refresh";
        return tokens;
    }();

    auto result = use_case->execute("john", "password");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(called);
    EXPECT_EQ("custom_token", result->access_token);
}

} // namespace inventory::test
