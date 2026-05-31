#include <gtest/gtest.h>
#include <memory>
#include <unordered_map>
#include "core/use_cases/update_item_use_case.hpp"

namespace inventory::test {

class MockUpdateItemRepository : public ItemRepository {
public:
    std::unordered_map<std::string, Item> items_by_id;

    Result<Item> find_by_id(const utils::Uuid& id) override
    {
        auto it = items_by_id.find(id.to_string());
        if (it != items_by_id.end()) {
            return it->second;
        }
        return std::unexpected(Error::not_found("Item not found"));
    }

    Result<Item> find_by_sku(const std::string&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<std::vector<Item>> find_all(const ItemFilter&) override
    {
        return std::vector<Item>{};
    }

    Result<int64_t> count(const ItemFilter&) override
    {
        return 0;
    }

    Result<Item> save(const Item&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<Item> update(const Item& item) override
    {
        items_by_id[item.id().to_string()] = item;
        return item;
    }

    Result<void> remove(const utils::Uuid&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<std::vector<Item>> find_low_stock() override
    {
        return std::vector<Item>{};
    }

    Item add_item(const Item& item)
    {
        items_by_id[item.id().to_string()] = item;
        return item;
    }
};

class TestUpdateItemUseCase : public ::testing::Test {
protected:
    std::shared_ptr<MockUpdateItemRepository> repo;
    std::unique_ptr<UpdateItemUseCase> use_case;
    utils::Uuid item_id;
    Item existing_item;

    void SetUp() override
    {
        repo = std::make_shared<MockUpdateItemRepository>();
        use_case = std::make_unique<UpdateItemUseCase>(repo);

        item_id = utils::Uuid::generate();
        auto now = std::chrono::system_clock::now();
        existing_item = Item(item_id, "SKU-300", "Original Name", "Original desc",
            utils::Uuid::generate(), utils::Uuid::generate(),
            Money::create(20000, "IDR").value(),
            Quantity::create(100, "pcs").value(),
            Quantity::create(10, "pcs").value(),
            ItemStatus::kActive, now, now);
        repo->add_item(existing_item);
    }
};

TEST_F(TestUpdateItemUseCase, UpdateName)
{
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.name = "Updated Name";

    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("Updated Name", result->name());
}

TEST_F(TestUpdateItemUseCase, UpdateDescription)
{
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.description = "Updated description";

    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("Updated description", result->description());
}

TEST_F(TestUpdateItemUseCase, UpdateUnitPrice)
{
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.unit_price_amount = 50000;

    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(50000, result->unit_price().amount());
}

TEST_F(TestUpdateItemUseCase, UpdateStock)
{
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.stock_quantity = 200;

    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(200, result->stock().value());
}

TEST_F(TestUpdateItemUseCase, UpdateMinStock)
{
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.min_stock_quantity = 25;

    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(25, result->min_stock().value());
}

TEST_F(TestUpdateItemUseCase, UpdateStatus)
{
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.status = ItemStatus::kInactive;

    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(ItemStatus::kInactive, result->status());
}

TEST_F(TestUpdateItemUseCase, UpdateCategoryId)
{
    auto new_cat = utils::Uuid::generate();
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.category_id = new_cat;

    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(new_cat, result->category_id());
}

TEST_F(TestUpdateItemUseCase, UpdateSupplierId)
{
    auto new_supp = utils::Uuid::generate();
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.supplier_id = new_supp;

    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(new_supp, result->supplier_id());
}

TEST_F(TestUpdateItemUseCase, UpdateMultipleFields)
{
    auto new_cat = utils::Uuid::generate();
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.name = "New Name";
    req.unit_price_amount = 75000;
    req.status = ItemStatus::kDiscontinued;

    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("New Name", result->name());
    EXPECT_EQ(75000, result->unit_price().amount());
    EXPECT_EQ(ItemStatus::kDiscontinued, result->status());
    EXPECT_EQ("Original desc", result->description());
}

TEST_F(TestUpdateItemUseCase, UpdateNonExistentItemFails)
{
    UpdateItemUseCase::Request req;
    req.id = utils::Uuid::generate();
    req.name = "Ghost";

    auto result = use_case->execute(req);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kNotFound, result.error().code());
}

TEST_F(TestUpdateItemUseCase, UpdateWithEmptyNameFails)
{
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.name = "";

    auto result = use_case->execute(req);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestUpdateItemUseCase, UpdateWithNegativeStockFails)
{
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.stock_quantity = -10;

    auto result = use_case->execute(req);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestUpdateItemUseCase, UpdateWithNegativePriceFails)
{
    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.unit_price_amount = -1;

    auto result = use_case->execute(req);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestUpdateItemUseCase, UpdateMarksUpdatedTimestamp)
{
    auto before = existing_item.updated_at();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    UpdateItemUseCase::Request req;
    req.id = item_id;
    req.name = "Timed Update";

    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_GT(result->updated_at(), before);
}

TEST_F(TestUpdateItemUseCase, NoUpdateWhenNoFieldsProvided)
{
    UpdateItemUseCase::Request req;
    req.id = item_id;

    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("Original Name", result->name());
    EXPECT_EQ(20000, result->unit_price().amount());
    EXPECT_EQ(ItemStatus::kActive, result->status());
}

} // namespace inventory::test
