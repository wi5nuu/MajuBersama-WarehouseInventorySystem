#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "core/use_cases/create_item_use_case.hpp"

namespace inventory::test {

class MockItemRepository : public ItemRepository {
public:
    std::vector<Item> items;

    Result<Item> find_by_id(const utils::Uuid&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<Item> find_by_sku(const std::string& sku) override
    {
        for (auto& item : items) {
            if (item.sku() == sku) {
                return item;
            }
        }
        return std::unexpected(Error::not_found("SKU not found"));
    }

    Result<std::vector<Item>> find_all(const ItemFilter&) override
    {
        return items;
    }

    Result<int64_t> count(const ItemFilter&) override
    {
        return static_cast<int64_t>(items.size());
    }

    Result<Item> save(const Item& item) override
    {
        items.push_back(item);
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

class TestCreateItemUseCase : public ::testing::Test {
protected:
    std::shared_ptr<MockItemRepository> repo;
    std::unique_ptr<CreateItemUseCase> use_case;

    void SetUp() override
    {
        repo = std::make_shared<MockItemRepository>();
        use_case = std::make_unique<CreateItemUseCase>(repo);
    }

    CreateItemUseCase::Request make_request() const
    {
        return CreateItemUseCase::Request{
            "SKU-100", "Test Item", "Description",
            utils::Uuid::generate(), utils::Uuid::generate(),
            50000, "IDR", 100, 10, "pcs"
        };
    }
};

TEST_F(TestCreateItemUseCase, SuccessfulCreation)
{
    auto req = make_request();
    auto result = use_case->execute(req);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("SKU-100", result->sku());
    EXPECT_EQ("Test Item", result->name());
    EXPECT_EQ(50000, result->unit_price().amount());
    EXPECT_EQ(100, result->stock().value());
    EXPECT_EQ(10, result->min_stock().value());
    EXPECT_EQ(ItemStatus::kActive, result->status());
}

TEST_F(TestCreateItemUseCase, DuplicateSkuFails)
{
    auto req = make_request();
    ASSERT_TRUE(use_case->execute(req).has_value());

    auto result = use_case->execute(req);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kConflict, result.error().code());
}

TEST_F(TestCreateItemUseCase, InvalidNegativePriceFails)
{
    auto req = make_request();
    req.unit_price_amount = -100;
    auto result = use_case->execute(req);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestCreateItemUseCase, InvalidNegativeStockFails)
{
    auto req = make_request();
    req.stock_quantity = -5;
    auto result = use_case->execute(req);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestCreateItemUseCase, InvalidNegativeMinStockFails)
{
    auto req = make_request();
    req.min_stock_quantity = -1;
    auto result = use_case->execute(req);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestCreateItemUseCase, CreatesItemWithGeneratedUuid)
{
    auto req = make_request();
    auto result = use_case->execute(req);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->id().is_valid());
}

TEST_F(TestCreateItemUseCase, MultipleItemsCanBeCreated)
{
    auto req1 = make_request();
    req1.sku = "SKU-100";
    auto req2 = make_request();
    req2.sku = "SKU-101";

    ASSERT_TRUE(use_case->execute(req1).has_value());
    ASSERT_TRUE(use_case->execute(req2).has_value());
    EXPECT_EQ(2, repo->items.size());
}

} // namespace inventory::test
