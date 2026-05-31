#pragma once

#include <chrono>
#include <string>

#include "../../common/utils/uuid.hpp"

namespace inventory {

class Category {
public:
    Category() = default;

    Category(
        utils::Uuid id,
        std::string name,
        std::string description,
        std::optional<utils::Uuid> parent_id,
        bool is_active,
        std::chrono::system_clock::time_point created_at,
        std::chrono::system_clock::time_point updated_at)
        : m_id(std::move(id))
        , m_name(std::move(name))
        , m_description(std::move(description))
        , m_parent_id(std::move(parent_id))
        , m_is_active(is_active)
        , m_created_at(created_at)
        , m_updated_at(updated_at)
    {
    }

    [[nodiscard]] const utils::Uuid& id() const noexcept { return m_id; }
    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    [[nodiscard]] const std::string& description() const noexcept { return m_description; }
    [[nodiscard]] const std::optional<utils::Uuid>& parent_id() const noexcept { return m_parent_id; }
    [[nodiscard]] bool is_active() const noexcept { return m_is_active; }
    [[nodiscard]] const auto& created_at() const noexcept { return m_created_at; }
    [[nodiscard]] const auto& updated_at() const noexcept { return m_updated_at; }

    void set_name(const std::string& name) { m_name = name; }
    void set_description(const std::string& desc) { m_description = desc; }
    void set_parent_id(const std::optional<utils::Uuid>& pid) { m_parent_id = pid; }
    void set_active(bool active) { m_is_active = active; }

private:
    utils::Uuid m_id;
    std::string m_name;
    std::string m_description;
    std::optional<utils::Uuid> m_parent_id;
    bool m_is_active{ true };
    std::chrono::system_clock::time_point m_created_at;
    std::chrono::system_clock::time_point m_updated_at;
};

} // namespace inventory
