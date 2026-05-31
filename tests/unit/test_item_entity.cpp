#include <gtest/gtest.h>
#include <chrono>
#include "core/entities/item.hpp"

namespace inventory::test {

class TestItemEntity : public ::testing::Test {
protected:
    utils::Uuid id = utils::Uuid::generate();
    utils::Uuid cat_id = utils::Uuid::generate();
    utils::Uuid supp_id = utils::Uuid::generate();
    Money price = Money::create(25000, "IDR").value();
    Quantity stock = Quantity::create(100, "pcs").value();
    Quantity min_stock = Quantity::create(10, "pcs").value();
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    Item create_default_item() const
    {
        return Item(id, "SKU-001", "Test Item", "A test item",
            cat_id, supp_id, price, stock, min_stock,
            ItemStatus::kActive, now, now);
    }
};

TEST_F(TestItemEntity, CreateItem)
{
    auto item = create_default_item();
    EXPECT_EQ(id, item.id());
    EXPECT_EQ("SKU-001", item.sku());
    EXPECT_EQ("Test Item", item.name());
    EXPECT_EQ("A test item", item.description());
    EXPECT_EQ(cat_id, item.category_id());
    EXPECT_EQ(supp_id, item.supplier_id());
    EXPECT_EQ(price, item.unit_price());
    EXPECT_EQ(stock, item.stock());
    EXPECT_EQ(min_stock, item.min_stock());
    EXPECT_EQ(ItemStatus::kActive, item.status());
}

TEST_F(TestItemEntity, SetName)
{
    auto item = create_default_item();
    item.set_name("Updated Name");
    EXPECT_EQ("Updated Name", item.name());
}

TEST_F(TestItemEntity, SetDescription)
{
    auto item = create_default_item();
    item.set_description("Updated description");
    EXPECT_EQ("Updated description", item.description());
}

TEST_F(TestItemEntity, SetUnitPrice)
{
    auto item = create_default_item();
    auto new_price = Money::create(50000, "IDR").value();
    item.set_unit_price(new_price);
    EXPECT_EQ(new_price, item.unit_price());
}

TEST_F(TestItemEntity, SetStock)
{
    auto item = create_default_item();
    auto new_stock = Quantity::create(200, "pcs").value();
    item.set_stock(new_stock);
    EXPECT_EQ(new_stock, item.stock());
}

TEST_F(TestItemEntity, SetMinStock)
{
    auto item = create_default_item();
    auto new_min = Quantity::create(20, "pcs").value();
    item.set_min_stock(new_min);
    EXPECT_EQ(new_min, item.min_stock());
}

TEST_F(TestItemEntity, SetStatus)
{
    auto item = create_default_item();
    item.set_status(ItemStatus::kDiscontinued);
    EXPECT_EQ(ItemStatus::kDiscontinued, item.status());
}

TEST_F(TestItemEntity, SetCategoryId)
{
    auto item = create_default_item();
    auto new_cat = utils::Uuid::generate();
    item.set_category_id(new_cat);
    EXPECT_EQ(new_cat, item.category_id());
}

TEST_F(TestItemEntity, SetSupplierId)
{
    auto item = create_default_item();
    auto new_supp = utils::Uuid::generate();
    item.set_supplier_id(new_supp);
    EXPECT_EQ(new_supp, item.supplier_id());
}

TEST_F(TestItemEntity, IsLowStockWhenStockEqualsMinStock)
{
    auto item = create_default_item();
    EXPECT_TRUE(item.is_low_stock());
}

TEST_F(TestItemEntity, IsLowStockWhenStockBelowMinStock)
{
    auto low_stock = Quantity::create(5, "pcs").value();
    auto item = Item(id, "SKU-002", "Low Item", "", cat_id, supp_id, price, low_stock, min_stock,
        ItemStatus::kActive, now, now);
    EXPECT_TRUE(item.is_low_stock());
}

TEST_F(TestItemEntity, IsNotLowStockWhenStockAboveMinStock)
{
    auto high_stock = Quantity::create(200, "pcs").value();
    auto item = Item(id, "SKU-003", "High Item", "", cat_id, supp_id, price, high_stock, min_stock,
        ItemStatus::kActive, now, now);
    EXPECT_FALSE(item.is_low_stock());
}

TEST_F(TestItemEntity, IsOutOfStockWhenStockIsEmpty)
{
    auto empty = Quantity::create(0, "pcs").value();
    auto item = Item(id, "SKU-004", "Empty Item", "", cat_id, supp_id, price, empty, min_stock,
        ItemStatus::kActive, now, now);
    EXPECT_TRUE(item.is_out_of_stock());
}

TEST_F(TestItemEntity, IsNotOutOfStockWhenStockIsPositive)
{
    auto item = create_default_item();
    EXPECT_FALSE(item.is_out_of_stock());
}

TEST_F(TestItemEntity, AvailableStockWhenActive)
{
    auto item = create_default_item();
    EXPECT_EQ(stock, item.available_stock());
}

TEST_F(TestItemEntity, AvailableStockWhenInactiveReturnsZero)
{
    auto item = Item(id, "SKU-005", "Inactive", "", cat_id, supp_id, price, stock, min_stock,
        ItemStatus::kInactive, now, now);
    EXPECT_EQ(0, item.available_stock().value());
}

TEST_F(TestItemEntity, AvailableStockWhenDiscontinuedReturnsZero)
{
    auto item = Item(id, "SKU-006", "Disc", "", cat_id, supp_id, price, stock, min_stock,
        ItemStatus::kDiscontinued, now, now);
    EXPECT_EQ(0, item.available_stock().value());
}

TEST_F(TestItemEntity, AvailableStockWhenOutOfStockStatusReturnsZero)
{
    auto item = Item(id, "SKU-007", "OOS", "", cat_id, supp_id, price, stock, min_stock,
        ItemStatus::kOutOfStock, now, now);
    EXPECT_EQ(0, item.available_stock().value());
}

TEST_F(TestItemEntity, MarkUpdatedUpdatesTimestamp)
{
    auto item = create_default_item();
    auto before = item.updated_at();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    item.mark_updated();
    auto after = item.updated_at();
    EXPECT_GT(after, before);
}

} // namespace inventory::test
