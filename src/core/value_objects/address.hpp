#pragma once

#include <string>

#include "../../common/result.hpp"

namespace inventory {

class Address {
public:
    Address() = default;

    static Result<Address> create(
        std::string street,
        std::string city,
        std::string province,
        std::string postal_code,
        std::string country = "Indonesia")
    {
        if (street.empty()) {
            return Error::validation("Street cannot be empty");
        }
        if (city.empty()) {
            return Error::validation("City cannot be empty");
        }
        if (postal_code.empty()) {
            return Error::validation("Postal code cannot be empty");
        }
        return Address(
            std::move(street),
            std::move(city),
            std::move(province),
            std::move(postal_code),
            std::move(country));
    }

    [[nodiscard]] const std::string& street() const noexcept { return m_street; }
    [[nodiscard]] const std::string& city() const noexcept { return m_city; }
    [[nodiscard]] const std::string& province() const noexcept { return m_province; }
    [[nodiscard]] const std::string& postal_code() const noexcept { return m_postal_code; }
    [[nodiscard]] const std::string& country() const noexcept { return m_country; }

    [[nodiscard]] std::string full_address() const
    {
        return m_street + ", " + m_city + ", " + m_province + " " + m_postal_code + ", " + m_country;
    }

    bool operator==(const Address& other) const = default;

private:
    Address(
        std::string street,
        std::string city,
        std::string province,
        std::string postal_code,
        std::string country)
        : m_street(std::move(street))
        , m_city(std::move(city))
        , m_province(std::move(province))
        , m_postal_code(std::move(postal_code))
        , m_country(std::move(country))
    {
    }

    std::string m_street;
    std::string m_city;
    std::string m_province;
    std::string m_postal_code;
    std::string m_country{ "Indonesia" };
};

} // namespace inventory
