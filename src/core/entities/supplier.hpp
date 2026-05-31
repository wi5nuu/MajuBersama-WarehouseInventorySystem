#pragma once

#include <chrono>
#include <string>

#include "../value_objects/address.hpp"
#include "../../common/utils/uuid.hpp"

namespace inventory {

class Supplier {
public:
    Supplier() = default;

    Supplier(
        utils::Uuid id,
        std::string code,
        std::string name,
        std::string contact_person,
        std::string phone,
        std::string email,
        Address address,
        bool is_active,
        std::chrono::system_clock::time_point created_at,
        std::chrono::system_clock::time_point updated_at)
        : m_id(std::move(id))
        , m_code(std::move(code))
        , m_name(std::move(name))
        , m_contact_person(std::move(contact_person))
        , m_phone(std::move(phone))
        , m_email(std::move(email))
        , m_address(std::move(address))
        , m_is_active(is_active)
        , m_created_at(created_at)
        , m_updated_at(updated_at)
    {
    }

    [[nodiscard]] const utils::Uuid& id() const noexcept { return m_id; }
    [[nodiscard]] const std::string& code() const noexcept { return m_code; }
    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    [[nodiscard]] const std::string& contact_person() const noexcept { return m_contact_person; }
    [[nodiscard]] const std::string& phone() const noexcept { return m_phone; }
    [[nodiscard]] const std::string& email() const noexcept { return m_email; }
    [[nodiscard]] const Address& address() const noexcept { return m_address; }
    [[nodiscard]] bool is_active() const noexcept { return m_is_active; }
    [[nodiscard]] const auto& created_at() const noexcept { return m_created_at; }
    [[nodiscard]] const auto& updated_at() const noexcept { return m_updated_at; }

    void set_name(const std::string& name) { m_name = name; }
    void set_contact_person(const std::string& cp) { m_contact_person = cp; }
    void set_phone(const std::string& phone) { m_phone = phone; }
    void set_email(const std::string& email) { m_email = email; }
    void set_address(const Address& addr) { m_address = addr; }
    void set_active(bool active) { m_is_active = active; }

private:
    utils::Uuid m_id;
    std::string m_code;
    std::string m_name;
    std::string m_contact_person;
    std::string m_phone;
    std::string m_email;
    Address m_address;
    bool m_is_active{ true };
    std::chrono::system_clock::time_point m_created_at;
    std::chrono::system_clock::time_point m_updated_at;
};

} // namespace inventory
