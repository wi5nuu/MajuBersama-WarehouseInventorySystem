#include <gtest/gtest.h>
#include "core/value_objects/email.hpp"

namespace inventory::test {

class TestEmail : public ::testing::Test {
};

TEST_F(TestEmail, CreateValidEmail)
{
    auto result = Email::create("user@example.com");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("user@example.com", result->value());
}

TEST_F(TestEmail, CreateEmailWithSubdomain)
{
    auto result = Email::create("user@mail.example.com");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("user@mail.example.com", result->value());
}

TEST_F(TestEmail, CreateEmailWithPlusTag)
{
    auto result = Email::create("user+tag@example.com");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("user+tag@example.com", result->value());
}

TEST_F(TestEmail, CreateEmailWithNumbers)
{
    auto result = Email::create("user123@example.com");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("user123@example.com", result->value());
}

TEST_F(TestEmail, CreateEmptyEmailFails)
{
    auto result = Email::create("");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestEmail, CreateEmailWithoutAtFails)
{
    auto result = Email::create("userexample.com");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestEmail, CreateEmailWithNoDomainFails)
{
    auto result = Email::create("user@");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestEmail, CreateEmailWithNoLocalPartFails)
{
    auto result = Email::create("@example.com");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestEmail, CreateEmailWithoutDotInDomainFails)
{
    auto result = Email::create("user@example");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestEmail, CreateEmailWithDotAtStartOfDomainFails)
{
    auto result = Email::create("user@.example.com");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestEmail, CreateEmailWithDotAtEndFails)
{
    auto result = Email::create("user@example.");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestEmail, CreateEmailTooLongFails)
{
    std::string local(250, 'a');
    std::string email = local + "@b.com";
    ASSERT_GT(email.size(), 254);
    auto result = Email::create(email);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestEmail, EqualityOperator)
{
    auto a = Email::create("test@example.com").value();
    auto b = Email::create("test@example.com").value();
    EXPECT_TRUE(a == b);
}

TEST_F(TestEmail, InequalityOperator)
{
    auto a = Email::create("test@example.com").value();
    auto b = Email::create("other@example.com").value();
    EXPECT_FALSE(a == b);
}

} // namespace inventory::test
