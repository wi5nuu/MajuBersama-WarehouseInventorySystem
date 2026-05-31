#pragma once

#include <chrono>
#include <string>

#include "core/entities/category.hpp"
#include "core/entities/item.hpp"
#include "core/entities/supplier.hpp"
#include "core/entities/user.hpp"
#include "core/entities/warehouse.hpp"
#include "core/value_objects/address.hpp"
#include "core/value_objects/email.hpp"
#include "core/value_objects/money.hpp"
#include "core/value_objects/quantity.hpp"

namespace inventory::test::fixtures {

/**
 * @brief Create a test Item with sensible defaults.
 */
inline Item create_test_item(
    utils::Uuid id = utils::Uuid::from_string("e0000000-0000-0000-0000-000000000001"),
    std::string sku = "SKU001",
    std::string name = "Test Item",
    std::string description = "A test item",
    utils::Uuid category_id = utils::Uuid::from_string("b0000000-0000-0000-0000-000000000001"),
    utils::Uuid supplier_id = utils::Uuid::from_string("c0000000-0000-0000-0000-000000000001"),
    Money unit_price = Money::create(15000000, "IDR").value(),
    Quantity stock = Quantity::create(50, "pcs").value(),
    Quantity min_stock = Quantity::create(10, "pcs").value(),
    ItemStatus status = ItemStatus::kActive)
{
    auto now = std::chrono::system_clock::now();
    return Item(
        std::move(id),
        std::move(sku),
        std::move(name),
        std::move(description),
        std::move(category_id),
        std::move(supplier_id),
        std::move(unit_price),
        std::move(stock),
        std::move(min_stock),
        status,
        now,
        now);
}

/**
 * @brief Create a test User with sensible defaults.
 */
inline User create_test_user(
    utils::Uuid id = utils::Uuid::from_string("a0000000-0000-0000-0000-000000000001"),
    std::string username = "testuser",
    Email email = Email::create("test@test.com").value(),
    std::string password_hash = "hash123",
    UserRole role = UserRole::kAdmin,
    bool is_active = true)
{
    auto now = std::chrono::system_clock::now();
    return User(
        std::move(id),
        std::move(username),
        std::move(email),
        std::move(password_hash),
        role,
        is_active,
        now,
        now);
}

/**
 * @brief Create a test Category with sensible defaults.
 */
inline Category create_test_category(
    utils::Uuid id = utils::Uuid::from_string("b0000000-0000-0000-0000-000000000001"),
    std::string name = "Test Category",
    std::string description = "A test category",
    bool is_active = true)
{
    auto now = std::chrono::system_clock::now();
    return Category(
        std::move(id),
        std::move(name),
        std::move(description),
        std::nullopt,
        is_active,
        now,
        now);
}

/**
 * @brief Create a test Supplier with sensible defaults.
 */
inline Supplier create_test_supplier(
    utils::Uuid id = utils::Uuid::from_string("c0000000-0000-0000-0000-000000000001"),
    std::string code = "SUP001",
    std::string name = "Test Supplier",
    std::string contact_person = "Contact Person",
    std::string phone = "021-1234",
    std::string email = "supplier@test.com",
    bool is_active = true)
{
    auto addr = Address::create("Jl. Test No. 1", "Jakarta", "DKI Jakarta", "12345", "Indonesia").value();
    auto now = std::chrono::system_clock::now();
    return Supplier(
        std::move(id),
        std::move(code),
        std::move(name),
        std::move(contact_person),
        std::move(phone),
        std::move(email),
        std::move(addr),
        is_active,
        now,
        now);
}

/**
 * @brief Create a test Warehouse with sensible defaults.
 */
inline Warehouse create_test_warehouse(
    utils::Uuid id = utils::Uuid::from_string("d0000000-0000-0000-0000-000000000001"),
    std::string code = "WH01",
    std::string name = "Test Warehouse",
    bool is_active = true)
{
    auto loc = Address::create("Jl. Warehouse No. 1", "Jakarta", "DKI Jakarta", "12345", "Indonesia").value();
    auto now = std::chrono::system_clock::now();
    return Warehouse(
        std::move(id),
        std::move(code),
        std::move(name),
        std::move(loc),
        is_active,
        now,
        now);
}

} // namespace inventory::test::fixtures
