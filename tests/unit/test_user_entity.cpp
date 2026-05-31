#include <gtest/gtest.h>
#include <chrono>
#include "core/entities/user.hpp"

namespace inventory::test {

class TestUserEntity : public ::testing::Test {
protected:
    utils::Uuid id = utils::Uuid::generate();
    Email email = Email::create("admin@example.com").value();
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    User create_user(UserRole role, bool active = true) const
    {
        return User(id, "testuser", email, "hashed_pwd",
            role, active, now, now);
    }
};

TEST_F(TestUserEntity, CreateUser)
{
    auto user = create_user(UserRole::kOperator);
    EXPECT_EQ(id, user.id());
    EXPECT_EQ("testuser", user.username());
    EXPECT_EQ(email, user.email());
    EXPECT_EQ("hashed_pwd", user.password_hash());
    EXPECT_EQ(UserRole::kOperator, user.role());
    EXPECT_TRUE(user.is_active());
}

TEST_F(TestUserEntity, SetPasswordHash)
{
    auto user = create_user(UserRole::kViewer);
    user.set_password_hash("new_hash");
    EXPECT_EQ("new_hash", user.password_hash());
}

TEST_F(TestUserEntity, SetRole)
{
    auto user = create_user(UserRole::kViewer);
    user.set_role(UserRole::kAdmin);
    EXPECT_EQ(UserRole::kAdmin, user.role());
}

TEST_F(TestUserEntity, SetActive)
{
    auto user = create_user(UserRole::kViewer);
    user.set_active(false);
    EXPECT_FALSE(user.is_active());
    user.set_active(true);
    EXPECT_TRUE(user.is_active());
}

class TestUserPermissions : public TestUserEntity {};

TEST_F(TestUserPermissions, AdminHasAllPermissions)
{
    auto user = create_user(UserRole::kAdmin);
    EXPECT_TRUE(user.has_permission("view"));
    EXPECT_TRUE(user.has_permission("create"));
    EXPECT_TRUE(user.has_permission("update"));
    EXPECT_TRUE(user.has_permission("delete"));
    EXPECT_TRUE(user.has_permission("unknown_permission"));
}

TEST_F(TestUserPermissions, ManagerHasViewCreateUpdateDelete)
{
    auto user = create_user(UserRole::kManager);
    EXPECT_TRUE(user.has_permission("view"));
    EXPECT_TRUE(user.has_permission("create"));
    EXPECT_TRUE(user.has_permission("update"));
    EXPECT_TRUE(user.has_permission("delete"));
    EXPECT_FALSE(user.has_permission("export"));
}

TEST_F(TestUserPermissions, OperatorHasViewCreateUpdate)
{
    auto user = create_user(UserRole::kOperator);
    EXPECT_TRUE(user.has_permission("view"));
    EXPECT_TRUE(user.has_permission("create"));
    EXPECT_TRUE(user.has_permission("update"));
    EXPECT_FALSE(user.has_permission("delete"));
    EXPECT_FALSE(user.has_permission("export"));
}

TEST_F(TestUserPermissions, ViewerHasViewOnly)
{
    auto user = create_user(UserRole::kViewer);
    EXPECT_TRUE(user.has_permission("view"));
    EXPECT_FALSE(user.has_permission("create"));
    EXPECT_FALSE(user.has_permission("update"));
    EXPECT_FALSE(user.has_permission("delete"));
}

TEST_F(TestUserPermissions, InactiveUserHasNoPermissions)
{
    auto user = create_user(UserRole::kAdmin, false);
    EXPECT_FALSE(user.has_permission("view"));
    EXPECT_FALSE(user.has_permission("create"));
    EXPECT_FALSE(user.has_permission("delete"));
}

class TestUserRoleConversion : public ::testing::Test {
};

TEST_F(TestUserRoleConversion, RoleToStringAdmin)
{
    EXPECT_EQ("admin", User::role_to_string(UserRole::kAdmin));
}

TEST_F(TestUserRoleConversion, RoleToStringManager)
{
    EXPECT_EQ("manager", User::role_to_string(UserRole::kManager));
}

TEST_F(TestUserRoleConversion, RoleToStringOperator)
{
    EXPECT_EQ("operator", User::role_to_string(UserRole::kOperator));
}

TEST_F(TestUserRoleConversion, RoleToStringViewer)
{
    EXPECT_EQ("viewer", User::role_to_string(UserRole::kViewer));
}

TEST_F(TestUserRoleConversion, StringToRoleAdmin)
{
    EXPECT_EQ(UserRole::kAdmin, User::string_to_role("admin"));
}

TEST_F(TestUserRoleConversion, StringToRoleManager)
{
    EXPECT_EQ(UserRole::kManager, User::string_to_role("manager"));
}

TEST_F(TestUserRoleConversion, StringToRoleOperator)
{
    EXPECT_EQ(UserRole::kOperator, User::string_to_role("operator"));
}

TEST_F(TestUserRoleConversion, StringToRoleViewer)
{
    EXPECT_EQ(UserRole::kViewer, User::string_to_role("viewer"));
}

TEST_F(TestUserRoleConversion, StringToRoleDefaultIsViewer)
{
    EXPECT_EQ(UserRole::kViewer, User::string_to_role("unknown"));
    EXPECT_EQ(UserRole::kViewer, User::string_to_role("superadmin"));
    EXPECT_EQ(UserRole::kViewer, User::string_to_role(""));
}

TEST_F(TestUserRoleConversion, RoundTrip)
{
    EXPECT_EQ(UserRole::kAdmin, User::string_to_role(User::role_to_string(UserRole::kAdmin)));
    EXPECT_EQ(UserRole::kManager, User::string_to_role(User::role_to_string(UserRole::kManager)));
    EXPECT_EQ(UserRole::kOperator, User::string_to_role(User::role_to_string(UserRole::kOperator)));
    EXPECT_EQ(UserRole::kViewer, User::string_to_role(User::role_to_string(UserRole::kViewer)));
}

} // namespace inventory::test
