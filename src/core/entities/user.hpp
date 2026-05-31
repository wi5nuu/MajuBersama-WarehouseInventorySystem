#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "../value_objects/email.hpp"
#include "../../common/utils/uuid.hpp"

namespace inventory {

enum class UserRole {
    kAdmin,
    kManager,
    kOperator,
    kViewer
};

class User {
public:
    User() = default;

    User(
        utils::Uuid id,
        std::string username,
        Email email,
        std::string password_hash,
        UserRole role,
        bool is_active,
        std::chrono::system_clock::time_point created_at,
        std::chrono::system_clock::time_point updated_at)
        : m_id(std::move(id))
        , m_username(std::move(username))
        , m_email(std::move(email))
        , m_password_hash(std::move(password_hash))
        , m_role(role)
        , m_is_active(is_active)
        , m_created_at(created_at)
        , m_updated_at(updated_at)
    {
    }

    [[nodiscard]] const utils::Uuid& id() const noexcept { return m_id; }
    [[nodiscard]] const std::string& username() const noexcept { return m_username; }
    [[nodiscard]] const Email& email() const noexcept { return m_email; }
    [[nodiscard]] const std::string& password_hash() const noexcept { return m_password_hash; }
    [[nodiscard]] UserRole role() const noexcept { return m_role; }
    [[nodiscard]] bool is_active() const noexcept { return m_is_active; }
    [[nodiscard]] const auto& created_at() const noexcept { return m_created_at; }
    [[nodiscard]] const auto& updated_at() const noexcept { return m_updated_at; }

    void set_password_hash(const std::string& hash) { m_password_hash = hash; }
    void set_role(UserRole role) { m_role = role; }
    void set_active(bool active) { m_is_active = active; }

    [[nodiscard]] bool has_permission(std::string_view permission) const
    {
        if (!m_is_active)
            return false;
        if (m_role == UserRole::kAdmin)
            return true;
        if (permission == "view" && m_role == UserRole::kViewer)
            return true;
        if ((permission == "view" || permission == "create" || permission == "update")
            && m_role == UserRole::kOperator)
            return true;
        if ((permission == "view" || permission == "create" || permission == "update" || permission == "delete")
            && m_role == UserRole::kManager)
            return true;
        return false;
    }

    [[nodiscard]] static std::string_view role_to_string(UserRole role)
    {
        switch (role) {
        case UserRole::kAdmin:
            return "admin";
        case UserRole::kManager:
            return "manager";
        case UserRole::kOperator:
            return "operator";
        case UserRole::kViewer:
            return "viewer";
        }
        return "unknown";
    }

    [[nodiscard]] static UserRole string_to_role(std::string_view s)
    {
        if (s == "admin")
            return UserRole::kAdmin;
        if (s == "manager")
            return UserRole::kManager;
        if (s == "operator")
            return UserRole::kOperator;
        return UserRole::kViewer;
    }

private:
    utils::Uuid m_id;
    std::string m_username;
    Email m_email;
    std::string m_password_hash;
    UserRole m_role{ UserRole::kViewer };
    bool m_is_active{ true };
    std::chrono::system_clock::time_point m_created_at;
    std::chrono::system_clock::time_point m_updated_at;
};

} // namespace inventory
