#pragma once

#include <concepts>
#include <expected>
#include <format>
#include <string>
#include <system_error>
#include <type_traits>
#include <variant>

namespace inventory {

enum class ErrorCode : int {
    kOk = 0,
    kNotFound = 1001,
    kValidationError = 1002,
    kUnauthorized = 1003,
    kForbidden = 1004,
    kConflict = 1005,
    kDatabaseError = 2001,
    kNetworkError = 2002,
    kTimeout = 2003,
    kInternalError = 5001,
    kConfigurationError = 5002,
    kAuthenticationFailed = 6001,
    kTokenExpired = 6002,
    kInvalidToken = 6003,
};

class Error {
public:
    Error(ErrorCode code, std::string message)
        : m_code(code)
        , m_message(std::move(message))
    {
    }

    [[nodiscard]] ErrorCode code() const noexcept { return m_code; }
    [[nodiscard]] const std::string& message() const noexcept { return m_message; }
    [[nodiscard]] std::string to_string() const
    {
        return std::format("[{}] {}", static_cast<int>(m_code), m_message);
    }

    static Error not_found(std::string msg = "Resource not found")
    {
        return Error(ErrorCode::kNotFound, std::move(msg));
    }

    static Error validation(std::string msg = "Validation failed")
    {
        return Error(ErrorCode::kValidationError, std::move(msg));
    }

    static Error unauthorized(std::string msg = "Unauthorized")
    {
        return Error(ErrorCode::kUnauthorized, std::move(msg));
    }

    static Error database(std::string msg = "Database error")
    {
        return Error(ErrorCode::kDatabaseError, std::move(msg));
    }

    static Error forbidden(std::string msg = "Forbidden")
    {
        return Error(ErrorCode::kForbidden, std::move(msg));
    }

    static Error conflict(std::string msg = "Conflict")
    {
        return Error(ErrorCode::kConflict, std::move(msg));
    }

    static Error internal(std::string msg = "Internal error")
    {
        return Error(ErrorCode::kInternalError, std::move(msg));
    }

    static Error configuration(std::string msg = "Configuration error")
    {
        return Error(ErrorCode::kConfigurationError, std::move(msg));
    }

private:
    ErrorCode m_code;
    std::string m_message;
};

template <typename T>
using Result = std::expected<T, Error>;

} // namespace inventory
