#pragma once

#include <cstdint>

#include "../../common/result.hpp"

namespace inventory {

class Quantity {
public:
    Quantity() = default;

    static Result<Quantity> create(std::int64_t value, std::string unit = "pcs")
    {
        if (value < 0) {
            return Error::validation("Quantity cannot be negative");
        }
        return Quantity(value, std::move(unit));
    }

    [[nodiscard]] std::int64_t value() const noexcept { return m_value; }
    [[nodiscard]] const std::string& unit() const noexcept { return m_unit; }

    [[nodiscard]] Quantity add(const Quantity& other) const
    {
        return Quantity(m_value + other.m_value, m_unit);
    }

    [[nodiscard]] Quantity subtract(const Quantity& other) const
    {
        auto result = m_value - other.m_value;
        if (result < 0)
            result = 0;
        return Quantity(result, m_unit);
    }

    [[nodiscard]] bool is_empty() const noexcept { return m_value == 0; }

    bool operator==(const Quantity& other) const = default;
    bool operator<(const Quantity& other) const { return m_value < other.m_value; }
    bool operator>(const Quantity& other) const { return m_value > other.m_value; }

private:
    Quantity(std::int64_t value, std::string unit)
        : m_value(value)
        , m_unit(std::move(unit))
    {
    }

    std::int64_t m_value{ 0 };
    std::string m_unit{ "pcs" };
};

} // namespace inventory
