#include <gtest/gtest.h>
#include <memory>
#include <unordered_map>
#include "core/use_cases/delete_item_use_case.hpp"

namespace inventory::test {

class MockDeleteItemRepository : public ItemRepository {
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

    Result<Item> update(const Item&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<void> remove(const utils::Uuid& id) override
    {
        auto it = items_by_id.find(id.to_string());
        if (it != items_by_id.end()) {
            items_by_id.erase(it);
            return {};
        }
        return std::unexpected(Error::not_found("Item not found"));
    }

    Result<std::vector<Item>> find_low_stock() override
    {
        return std::vector<Item>{};
    }

    void add_item(const Item& item)
    {
        items_by_id[item.id().to_string()] = item;
    }
};

class TestDeleteItemUseCase : public ::testing::Test {
protected:
    std::shared_ptr<MockDeleteItemRepository> repo;
    std::unique_ptr<DeleteItemUseCase> use_case;

    void SetUp() override
    {
        repo = std::make_shared<MockDeleteItemRepository>();
        use_case = std::make_unique<DeleteItemUseCase>(repo);
    }

    Item create_and_add_item(const std::string& sku)
    {
        auto now = std::chrono::system_clock::now();
        Item item(utils::Uuid::generate(), sku, "Item " + sku, "",
            utils::Uuid::generate(), utils::Uuid::generate(),
            Money::create(10000, "IDR").value(),
            Quantity::create(10, "pcs").value(),
            Quantity::create(2, "pcs").value(),
            ItemStatus::kActive, now, now);
        repo->add_item(item);
        return item;
    }
};

TEST_F(TestDeleteItemUseCase, DeleteExistingItem)
{
    auto item = create_and_add_item("SKU-400");
    EXPECT_EQ(1, repo->items_by_id.size());

    auto result = use_case->execute(item.id());
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(0, repo->items_by_id.size());
}

TEST_F(TestDeleteItemUseCase, DeleteNonExistentItemFails)
{
    auto id = utils::Uuid::generate();
    auto result = use_case->execute(id);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kNotFound, result.error().code());
}

TEST_F(TestDeleteItemUseCase, DeleteWithInvalidIdFails)
{
    utils::Uuid invalid_id;
    auto result = use_case->execute(invalid_id);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestDeleteItemUseCase, DeleteMultipleItemsIndividually)
{
    auto item1 = create_and_add_item("SKU-401");
    auto item2 = create_and_add_item("SKU-402");
    auto item3 = create_and_add_item("SKU-403");
    EXPECT_EQ(3, repo->items_by_id.size());

    ASSERT_TRUE(use_case->execute(item2.id()).has_value());
    EXPECT_EQ(2, repo->items_by_id.size());

    ASSERT_TRUE(use_case->execute(item1.id()).has_value());
    EXPECT_EQ(1, repo->items_by_id.size());

    ASSERT_TRUE(use_case->execute(item3.id()).has_value());
    EXPECT_EQ(0, repo->items_by_id.size());
}

TEST_F(TestDeleteItemUseCase, DeleteSameItemTwiceFailsSecondTime)
{
    auto item = create_and_add_item("SKU-404");

    auto first = use_case->execute(item.id());
    ASSERT_TRUE(first.has_value());

    auto second = use_case->execute(item.id());
    ASSERT_FALSE(second.has_value());
    EXPECT_EQ(ErrorCode::kNotFound, second.error().code());
}

} // namespace inventory::test
