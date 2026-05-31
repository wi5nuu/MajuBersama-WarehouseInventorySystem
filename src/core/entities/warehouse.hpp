#pragma once

#include <chrono>
#include <string>

#include "../value_objects/address.hpp"
#include "../../common/utils/uuid.hpp"

namespace inventory {

class Warehouse {
public:
    Warehouse() = default;

    Warehouse(
        utils::Uuid id,
        std::string code,
        std::string name,
        Address location,
        bool is_active,
        std::chrono::system_clock::time_point created_at,
        std::chrono::system_clock::time_point updated_at)
        : m_id(std::move(id))
        , m_code(std::move(code))
        , m_name(std::move(name))
        , m_location(std::move(location))
        , m_is_active(is_active)
        , m_created_at(created_at)
        , m_updated_at(updated_at)
    {
    }

    [[nodiscard]] const utils::Uuid& id() const noexcept { return m_id; }
    [[nodiscard]] const std::string& code() const noexcept { return m_code; }
    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    [[nodiscard]] const Address& location() const noexcept { return m_location; }
    [[nodiscard]] bool is_active() const noexcept { return m_is_active; }
    [[nodiscard]] const auto& created_at() const noexcept { return m_created_at; }
    [[nodiscard]] const auto& updated_at() const noexcept { return m_updated_at; }

    void set_name(const std::string& name) { m_name = name; }
    void set_location(const Address& loc) { m_location = loc; }
    void set_active(bool active) { m_is_active = active; }

private:
    utils::Uuid m_id;
    std::string m_code;
    std::string m_name;
    Address m_location;
    bool m_is_active{ true };
    std::chrono::system_clock::time_point m_created_at;
    std::chrono::system_clock::time_point m_updated_at;
};

} // namespace inventory
