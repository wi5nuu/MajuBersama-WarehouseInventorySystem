#include <gtest/gtest.h>
#include <string>
#include "common/result.hpp"

namespace inventory::test {

class TestResult : public ::testing::Test {
};

TEST_F(TestResult, ErrorConstruction)
{
    Error err(ErrorCode::kNotFound, "Item not found");
    EXPECT_EQ(ErrorCode::kNotFound, err.code());
    EXPECT_EQ("Item not found", err.message());
}

TEST_F(TestResult, ErrorToString)
{
    Error err(ErrorCode::kValidationError, "Invalid input");
    std::string expected = "[1002] Invalid input";
    EXPECT_EQ(expected, err.to_string());
}

TEST_F(TestResult, ErrorStaticNotFound)
{
    auto err = Error::not_found("Custom not found");
    EXPECT_EQ(ErrorCode::kNotFound, err.code());
    EXPECT_EQ("Custom not found", err.message());
}

TEST_F(TestResult, ErrorStaticNotFoundDefault)
{
    auto err = Error::not_found();
    EXPECT_EQ(ErrorCode::kNotFound, err.code());
    EXPECT_EQ("Resource not found", err.message());
}

TEST_F(TestResult, ErrorStaticValidation)
{
    auto err = Error::validation("Invalid SKU");
    EXPECT_EQ(ErrorCode::kValidationError, err.code());
    EXPECT_EQ("Invalid SKU", err.message());
}

TEST_F(TestResult, ErrorStaticValidationDefault)
{
    auto err = Error::validation();
    EXPECT_EQ(ErrorCode::kValidationError, err.code());
    EXPECT_EQ("Validation failed", err.message());
}

TEST_F(TestResult, ErrorStaticUnauthorized)
{
    auto err = Error::unauthorized("Login required");
    EXPECT_EQ(ErrorCode::kUnauthorized, err.code());
    EXPECT_EQ("Login required", err.message());
}

TEST_F(TestResult, ErrorStaticUnauthorizedDefault)
{
    auto err = Error::unauthorized();
    EXPECT_EQ(ErrorCode::kUnauthorized, err.code());
    EXPECT_EQ("Unauthorized", err.message());
}

TEST_F(TestResult, ErrorStaticForbidden)
{
    auto err = Error::forbidden("Access denied");
    EXPECT_EQ(ErrorCode::kForbidden, err.code());
    EXPECT_EQ("Access denied", err.message());
}

TEST_F(TestResult, ErrorStaticConflict)
{
    auto err = Error::conflict("Duplicate entry");
    EXPECT_EQ(ErrorCode::kConflict, err.code());
    EXPECT_EQ("Duplicate entry", err.message());
}

TEST_F(TestResult, ErrorStaticDatabase)
{
    auto err = Error::database("Connection lost");
    EXPECT_EQ(ErrorCode::kDatabaseError, err.code());
    EXPECT_EQ("Connection lost", err.message());
}

TEST_F(TestResult, ErrorStaticInternal)
{
    auto err = Error::internal("Something broke");
    EXPECT_EQ(ErrorCode::kInternalError, err.code());
    EXPECT_EQ("Something broke", err.message());
}

TEST_F(TestResult, ErrorStaticConfiguration)
{
    auto err = Error::configuration("Bad config");
    EXPECT_EQ(ErrorCode::kConfigurationError, err.code());
    EXPECT_EQ("Bad config", err.message());
}

TEST_F(TestResult, ResultWithSuccessValue)
{
    Result<int> result = 42;
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(42, result.value());
}

TEST_F(TestResult, ResultWithError)
{
    Result<int> result = std::unexpected(Error::not_found("Missing"));
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kNotFound, result.error().code());
}

TEST_F(TestResult, ResultErrorPropagation)
{
    auto fn = []() -> Result<std::string> {
        return std::unexpected(Error::validation("Bad input"));
    };
    auto result = fn();
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ("Bad input", result.error().message());
}

TEST_F(TestResult, ResultValueAccess)
{
    Result<std::string> result("hello");
    EXPECT_EQ("hello", *result);
}

TEST_F(TestResult, ErrorCodeValues)
{
    EXPECT_EQ(0, static_cast<int>(ErrorCode::kOk));
    EXPECT_EQ(1001, static_cast<int>(ErrorCode::kNotFound));
    EXPECT_EQ(1002, static_cast<int>(ErrorCode::kValidationError));
    EXPECT_EQ(1003, static_cast<int>(ErrorCode::kUnauthorized));
    EXPECT_EQ(1004, static_cast<int>(ErrorCode::kForbidden));
    EXPECT_EQ(1005, static_cast<int>(ErrorCode::kConflict));
    EXPECT_EQ(2001, static_cast<int>(ErrorCode::kDatabaseError));
    EXPECT_EQ(2002, static_cast<int>(ErrorCode::kNetworkError));
    EXPECT_EQ(2003, static_cast<int>(ErrorCode::kTimeout));
    EXPECT_EQ(5001, static_cast<int>(ErrorCode::kInternalError));
    EXPECT_EQ(5002, static_cast<int>(ErrorCode::kConfigurationError));
    EXPECT_EQ(6001, static_cast<int>(ErrorCode::kAuthenticationFailed));
    EXPECT_EQ(6002, static_cast<int>(ErrorCode::kTokenExpired));
    EXPECT_EQ(6003, static_cast<int>(ErrorCode::kInvalidToken));
}

} // namespace inventory::test
