#include <gtest/gtest.h>
#include <memory>
#include <unordered_map>
#include "core/use_cases/get_item_use_case.hpp"

namespace inventory::test {

class MockGetItemRepository : public ItemRepository {
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

    Result<Item> save(const Item& item) override
    {
        items_by_id[item.id().to_string()] = item;
        return item;
    }

    Result<Item> update(const Item&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<void> remove(const utils::Uuid&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<std::vector<Item>> find_low_stock() override
    {
        return std::vector<Item>{};
    }
};

class TestGetItemUseCase : public ::testing::Test {
protected:
    std::shared_ptr<MockGetItemRepository> repo;
    std::unique_ptr<GetItemUseCase> use_case;

    void SetUp() override
    {
        repo = std::make_shared<MockGetItemRepository>();
        use_case = std::make_unique<GetItemUseCase>(repo);
    }
};

TEST_F(TestGetItemUseCase, GetExistingItem)
{
    auto id = utils::Uuid::generate();
    auto now = std::chrono::system_clock::now();
    auto price = Money::create(15000, "IDR").value();
    auto qty = Quantity::create(50, "pcs").value();
    Item item(id, "SKU-200", "Gadget", "A gadget", utils::Uuid::generate(),
        utils::Uuid::generate(), price, qty, qty,
        ItemStatus::kActive, now, now);
    repo->save(item);

    auto result = use_case->execute(id);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("SKU-200", result->sku());
    EXPECT_EQ("Gadget", result->name());
}

TEST_F(TestGetItemUseCase, GetNonExistentItemFails)
{
    auto id = utils::Uuid::generate();
    auto result = use_case->execute(id);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kNotFound, result.error().code());
}

TEST_F(TestGetItemUseCase, GetWithInvalidIdFails)
{
    utils::Uuid invalid_id;
    auto result = use_case->execute(invalid_id);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestGetItemUseCase, GetReturnsCorrectItemWhenMultipleExist)
{
    auto now = std::chrono::system_clock::now();
    auto price = Money::create(10000, "IDR").value();
    auto qty = Quantity::create(10, "pcs").value();

    auto id1 = utils::Uuid::generate();
    auto id2 = utils::Uuid::generate();

    Item item1(id1, "SKU-A", "Item A", "", utils::Uuid::generate(),
        utils::Uuid::generate(), price, qty, qty, ItemStatus::kActive, now, now);
    Item item2(id2, "SKU-B", "Item B", "", utils::Uuid::generate(),
        utils::Uuid::generate(), price, qty, qty, ItemStatus::kActive, now, now);

    repo->save(item1);
    repo->save(item2);

    auto result = use_case->execute(id2);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("SKU-B", result->sku());
}

} // namespace inventory::test
