#pragma once

#include <chrono>
#include <string>

#include "../value_objects/money.hpp"
#include "../value_objects/quantity.hpp"
#include "../../common/utils/uuid.hpp"

namespace inventory {

enum class ItemStatus {
    kActive,
    kInactive,
    kDiscontinued,
    kOutOfStock
};

class Item {
public:
    Item() = default;

    Item(
        utils::Uuid id,
        std::string sku,
        std::string name,
        std::string description,
        utils::Uuid category_id,
        utils::Uuid supplier_id,
        Money unit_price,
        Quantity stock,
        Quantity min_stock,
        ItemStatus status,
        std::chrono::system_clock::time_point created_at,
        std::chrono::system_clock::time_point updated_at)
        : m_id(std::move(id))
        , m_sku(std::move(sku))
        , m_name(std::move(name))
        , m_description(std::move(description))
        , m_category_id(std::move(category_id))
        , m_supplier_id(std::move(supplier_id))
        , m_unit_price(std::move(unit_price))
        , m_stock(std::move(stock))
        , m_min_stock(std::move(min_stock))
        , m_status(status)
        , m_created_at(created_at)
        , m_updated_at(updated_at)
    {
    }

    [[nodiscard]] const utils::Uuid& id() const noexcept { return m_id; }
    [[nodiscard]] const std::string& sku() const noexcept { return m_sku; }
    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    [[nodiscard]] const std::string& description() const noexcept { return m_description; }
    [[nodiscard]] const utils::Uuid& category_id() const noexcept { return m_category_id; }
    [[nodiscard]] const utils::Uuid& supplier_id() const noexcept { return m_supplier_id; }
    [[nodiscard]] const Money& unit_price() const noexcept { return m_unit_price; }
    [[nodiscard]] const Quantity& stock() const noexcept { return m_stock; }
    [[nodiscard]] const Quantity& min_stock() const noexcept { return m_min_stock; }
    [[nodiscard]] ItemStatus status() const noexcept { return m_status; }
    [[nodiscard]] const auto& created_at() const noexcept { return m_created_at; }
    [[nodiscard]] const auto& updated_at() const noexcept { return m_updated_at; }

    void set_name(const std::string& name) { m_name = name; }
    void set_description(const std::string& desc) { m_description = desc; }
    void set_unit_price(const Money& price) { m_unit_price = price; }
    void set_stock(const Quantity& qty) { m_stock = qty; }
    void set_min_stock(const Quantity& qty) { m_min_stock = qty; }
    void set_status(ItemStatus s) { m_status = s; }
    void set_category_id(const utils::Uuid& cid) { m_category_id = cid; }
    void set_supplier_id(const utils::Uuid& sid) { m_supplier_id = sid; }
    void mark_updated() { m_updated_at = std::chrono::system_clock::now(); }

    [[nodiscard]] bool is_low_stock() const
    {
        return m_stock.value() <= m_min_stock.value();
    }

    [[nodiscard]] bool is_out_of_stock() const
    {
        return m_stock.is_empty();
    }

    [[nodiscard]] Quantity available_stock() const
    {
        if (m_status == ItemStatus::kActive) {
            return m_stock;
        }
        return Quantity::create(0).value();
    }

private:
    utils::Uuid m_id;
    std::string m_sku;
    std::string m_name;
    std::string m_description;
    utils::Uuid m_category_id;
    utils::Uuid m_supplier_id;
    Money m_unit_price;
    Quantity m_stock;
    Quantity m_min_stock;
    ItemStatus m_status{ ItemStatus::kActive };
    std::chrono::system_clock::time_point m_created_at;
    std::chrono::system_clock::time_point m_updated_at;
};

} // namespace inventory
