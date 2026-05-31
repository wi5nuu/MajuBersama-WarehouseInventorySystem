#pragma once

#include <cstdint>
#include <format>
#include <string>

#include "../../common/result.hpp"

namespace inventory {

class Money {
public:
    Money() = default;

    static Result<Money> create(std::int64_t amount, std::string currency = "IDR")
    {
        if (amount < 0) {
            return Error::validation("Amount cannot be negative");
        }
        if (currency.size() != 3) {
            return Error::validation("Currency must be a 3-letter ISO code");
        }
        return Money(amount, std::move(currency));
    }

    [[nodiscard]] std::int64_t amount() const noexcept { return m_amount; }
    [[nodiscard]] const std::string& currency() const noexcept { return m_currency; }

    [[nodiscard]] Money add(const Money& other) const
    {
        return Money(m_amount + other.m_amount, m_currency);
    }

    [[nodiscard]] Money subtract(const Money& other) const
    {
        return Money(m_amount - other.m_amount, m_currency);
    }

    [[nodiscard]] Money multiply(double factor) const
    {
        return Money(static_cast<std::int64_t>(m_amount * factor), m_currency);
    }

    [[nodiscard]] std::string to_string() const
    {
        auto major = m_amount / 100;
        auto minor = m_amount % 100;
        return std::format("{}.{:02d} {}", major, minor, m_currency);
    }

    bool operator==(const Money& other) const = default;

private:
    Money(std::int64_t amount, std::string currency)
        : m_amount(amount)
        , m_currency(std::move(currency))
    {
    }

    std::int64_t m_amount{ 0 };
    std::string m_currency{ "IDR" };
};

} // namespace inventory
