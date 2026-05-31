/**
 * @file test_item_repository.cpp
 * @brief Integration tests for PostgresItemRepository.
 *
 * These tests require a live PostgreSQL database and are DISABLED by default.
 * To run them, remove the DISABLED_ prefix or filter with --gtest_also_run_disabled_tests.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <vector>

#include "core/entities/item.hpp"
#include "core/repositories/item_repository.hpp"
#include "core/value_objects/money.hpp"
#include "core/value_objects/quantity.hpp"
#include "infrastructure/database/connection_pool.hpp"
#include "infrastructure/repositories/postgres_item_repository.hpp"
#include "tests/fixtures/mock_data.hpp"

namespace inventory::test {

class DISABLED_PostgresItemRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_pool = std::make_shared<ConnectionPool>(
            2,
            "localhost",
            5432,
            "inventory_test",
            "test_user",
            "test_pass");

        m_repo = std::make_unique<PostgresItemRepository>(*m_pool);
    }

    void TearDown() override
    {
        m_repo.reset();
        m_pool.reset();
    }

    std::shared_ptr<ConnectionPool> m_pool;
    std::unique_ptr<PostgresItemRepository> m_repo;
};

/**
 * @brief Test saving a new item and then finding it by ID.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, SaveAndFindById)
{
    auto item = fixtures::create_test_item();
    auto save_result = m_repo->save(item);
    ASSERT_TRUE(save_result) << "Save failed: " << save_result.error().message();

    auto find_result = m_repo->find_by_id(item.id());
    ASSERT_TRUE(find_result) << "Find by ID failed: " << find_result.error().message();
    EXPECT_EQ(find_result->sku(), item.sku());
    EXPECT_EQ(find_result->name(), item.name());
    EXPECT_EQ(find_result->unit_price(), item.unit_price());
    EXPECT_EQ(find_result->stock(), item.stock());
    EXPECT_EQ(find_result->status(), item.status());
}

/**
 * @brief Test finding an item by its SKU.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, FindBySku)
{
    auto item = fixtures::create_test_item();
    auto save_result = m_repo->save(item);
    ASSERT_TRUE(save_result);

    auto find_result = m_repo->find_by_sku(item.sku());
    ASSERT_TRUE(find_result) << "Find by SKU failed: " << find_result.error().message();
    EXPECT_EQ(find_result->id(), item.id());
    EXPECT_EQ(find_result->name(), item.name());
}

/**
 * @brief Test that find_by_id returns kNotFound for a non-existent item.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, FindByIdNotFound)
{
    auto unknown_id = utils::Uuid::generate();
    auto result = m_repo->find_by_id(unknown_id);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code(), ErrorCode::kNotFound);
}

/**
 * @brief Test that find_by_sku returns kNotFound for a non-existent SKU.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, FindBySkuNotFound)
{
    auto result = m_repo->find_by_sku("NONEXISTENT_SKU");
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code(), ErrorCode::kNotFound);
}

/**
 * @brief Test updating an existing item's name and price.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, UpdateItem)
{
    auto item = fixtures::create_test_item();
    auto save_result = m_repo->save(item);
    ASSERT_TRUE(save_result);

    auto fetched = m_repo->find_by_id(item.id());
    ASSERT_TRUE(fetched);
    fetched->set_name("Updated Item Name");
    auto new_price = Money::create(20000000, "IDR").value();
    fetched->set_unit_price(new_price);
    fetched->set_status(ItemStatus::kDiscontinued);

    auto update_result = m_repo->update(*fetched);
    ASSERT_TRUE(update_result) << "Update failed: " << update_result.error().message();
    EXPECT_EQ(update_result->name(), "Updated Item Name");
    EXPECT_EQ(update_result->unit_price(), new_price);
    EXPECT_EQ(update_result->status(), ItemStatus::kDiscontinued);
}

/**
 * @brief Test deleting an item and verifying it is removed.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, RemoveItem)
{
    auto item = fixtures::create_test_item();
    auto save_result = m_repo->save(item);
    ASSERT_TRUE(save_result);

    auto remove_result = m_repo->remove(item.id());
    ASSERT_TRUE(remove_result) << "Remove failed: " << remove_result.error().message();

    auto find_result = m_repo->find_by_id(item.id());
    ASSERT_FALSE(find_result);
    EXPECT_EQ(find_result.error().code(), ErrorCode::kNotFound);
}

/**
 * @brief Test find_all with no filter returns all items.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, FindAllNoFilter)
{
    auto item1 = fixtures::create_test_item(
        utils::Uuid::generate(), "SKU-FA-1", "FindAll Item 1");
    auto item2 = fixtures::create_test_item(
        utils::Uuid::generate(), "SKU-FA-2", "FindAll Item 2");

    ASSERT_TRUE(m_repo->save(item1));
    ASSERT_TRUE(m_repo->save(item2));

    ItemFilter filter;
    filter.page_size = 50;
    auto results = m_repo->find_all(filter);
    ASSERT_TRUE(results);
    EXPECT_GE(results->size(), static_cast<std::size_t>(2));
}

/**
 * @brief Test find_all with a category filter.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, FindAllByCategory)
{
    auto cat_a = utils::Uuid::from_string("b0000000-0000-0000-0000-000000000001");
    auto cat_b = utils::Uuid::from_string("b0000000-0000-0000-0000-000000000002");

    auto item_a = fixtures::create_test_item(
        utils::Uuid::generate(), "SKU-CAT-1", "Cat A Item", "desc", cat_a);
    auto item_b = fixtures::create_test_item(
        utils::Uuid::generate(), "SKU-CAT-2", "Cat B Item", "desc", cat_b);

    ASSERT_TRUE(m_repo->save(item_a));
    ASSERT_TRUE(m_repo->save(item_b));

    ItemFilter filter;
    filter.category_id = cat_a;
    filter.page_size = 50;

    auto results = m_repo->find_all(filter);
    ASSERT_TRUE(results);
    for (const auto& it : *results) {
        EXPECT_EQ(it.category_id(), cat_a);
    }
}

/**
 * @brief Test count with no filter.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, CountAll)
{
    auto item = fixtures::create_test_item(
        utils::Uuid::generate(), "SKU-COUNT", "Count Item");
    ASSERT_TRUE(m_repo->save(item));

    ItemFilter filter;
    auto count_result = m_repo->count(filter);
    ASSERT_TRUE(count_result);
    EXPECT_GT(*count_result, 0);
}

/**
 * @brief Test find_low_stock returns items whose stock is at or below minimum.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, FindLowStock)
{
    auto low_item = fixtures::create_test_item(
        utils::Uuid::generate(),
        "SKU-LOW",
        "Low Stock Item",
        "desc",
        utils::Uuid::from_string("b0000000-0000-0000-0000-000000000001"),
        utils::Uuid::from_string("c0000000-0000-0000-0000-000000000001"),
        Money::create(100000, "IDR").value(),
        Quantity::create(5, "pcs").value(),
        Quantity::create(10, "pcs").value());

    ASSERT_TRUE(m_repo->save(low_item));

    auto low_stock_results = m_repo->find_low_stock();
    ASSERT_TRUE(low_stock_results);
    bool found = false;
    for (const auto& it : *low_stock_results) {
        if (it.sku() == "SKU-LOW") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Low stock item not found in find_low_stock results";
}

/**
 * @brief Test that updating a non-existent item returns kNotFound.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, UpdateNonExistentItem)
{
    auto fake_item = fixtures::create_test_item(utils::Uuid::generate(), "SKU-FAKE");
    auto result = m_repo->update(fake_item);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code(), ErrorCode::kNotFound);
}

/**
 * @brief Test that removing a non-existent item returns kNotFound.
 */
TEST_F(DISABLED_PostgresItemRepositoryTest, RemoveNonExistentItem)
{
    auto fake_id = utils::Uuid::generate();
    auto result = m_repo->remove(fake_id);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code(), ErrorCode::kNotFound);
}

} // namespace inventory::test
