#pragma once

#include <string>

#include "../../common/result.hpp"

namespace inventory {

class Email {
public:
    Email() = default;

    static Result<Email> create(std::string value)
    {
        if (!is_valid(value)) {
            return Error::validation("Invalid email address");
        }
        return Email(std::move(value));
    }

    [[nodiscard]] const std::string& value() const noexcept { return m_value; }

    bool operator==(const Email& other) const = default;

private:
    explicit Email(std::string value)
        : m_value(std::move(value))
    {
    }

    static bool is_valid(const std::string& email)
    {
        if (email.empty() || email.size() > 254)
            return false;
        auto at = email.find('@');
        if (at == std::string::npos || at == 0 || at == email.size() - 1)
            return false;
        auto dot = email.find('.', at + 1);
        if (dot == std::string::npos || dot == at + 1 || dot == email.size() - 1)
            return false;
        return true;
    }

    std::string m_value;
};

} // namespace inventory
