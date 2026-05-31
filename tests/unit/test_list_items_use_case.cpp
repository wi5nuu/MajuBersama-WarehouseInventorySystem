#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "core/use_cases/list_items_use_case.hpp"

namespace inventory::test {

class MockListItemRepository : public ItemRepository {
public:
    std::vector<Item> items;

    Result<Item> find_by_id(const utils::Uuid&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<Item> find_by_sku(const std::string&) override
    {
        return std::unexpected(Error::not_found());
    }

    Result<std::vector<Item>> find_all(const ItemFilter& filter) override
    {
        size_t start = (filter.page - 1) * filter.page_size;
        size_t end = std::min(start + static_cast<size_t>(filter.page_size), items.size());
        if (start >= items.size()) {
            return std::vector<Item>{};
        }
        return std::vector<Item>(items.begin() + start, items.begin() + end);
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

class TestListItemsUseCase : public ::testing::Test {
protected:
    std::shared_ptr<MockListItemRepository> repo;
    std::unique_ptr<ListItemsUseCase> use_case;

    void SetUp() override
    {
        repo = std::make_shared<MockListItemRepository>();
        use_case = std::make_unique<ListItemsUseCase>(repo);
    }

    Item create_item(const std::string& sku) const
    {
        auto now = std::chrono::system_clock::now();
        return Item(utils::Uuid::generate(), sku, "Item " + sku, "",
            utils::Uuid::generate(), utils::Uuid::generate(),
            Money::create(10000, "IDR").value(),
            Quantity::create(10, "pcs").value(),
            Quantity::create(2, "pcs").value(),
            ItemStatus::kActive, now, now);
    }
};

TEST_F(TestListItemsUseCase, ListEmptyRepository)
{
    ItemFilter filter;
    auto result = use_case->execute(filter);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->items.empty());
    EXPECT_EQ(0, result->total_count);
    EXPECT_EQ(1, result->page);
    EXPECT_EQ(20, result->page_size);
    EXPECT_EQ(0, result->total_pages);
}

TEST_F(TestListItemsUseCase, ListAllItems)
{
    repo->save(create_item("SKU-001"));
    repo->save(create_item("SKU-002"));
    repo->save(create_item("SKU-003"));

    ItemFilter filter;
    filter.page_size = 10;
    auto result = use_case->execute(filter);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(3, result->items.size());
    EXPECT_EQ(3, result->total_count);
    EXPECT_EQ(1, result->total_pages);
}

TEST_F(TestListItemsUseCase, ListWithPaginationPage1)
{
    for (int i = 1; i <= 25; ++i) {
        auto sku = "SKU-" + std::to_string(100 + i);
        repo->save(create_item(sku));
    }

    ItemFilter filter;
    filter.page = 1;
    filter.page_size = 10;
    auto result = use_case->execute(filter);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(10, result->items.size());
    EXPECT_EQ(25, result->total_count);
    EXPECT_EQ(1, result->page);
    EXPECT_EQ(3, result->total_pages);
}

TEST_F(TestListItemsUseCase, ListWithPaginationPage2)
{
    for (int i = 1; i <= 25; ++i) {
        auto sku = "SKU-" + std::to_string(100 + i);
        repo->save(create_item(sku));
    }

    ItemFilter filter;
    filter.page = 2;
    filter.page_size = 10;
    auto result = use_case->execute(filter);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(10, result->items.size());
    EXPECT_EQ(25, result->total_count);
    EXPECT_EQ(2, result->page);
}

TEST_F(TestListItemsUseCase, ListLastPageWithPartialResults)
{
    for (int i = 1; i <= 25; ++i) {
        auto sku = "SKU-" + std::to_string(100 + i);
        repo->save(create_item(sku));
    }

    ItemFilter filter;
    filter.page = 3;
    filter.page_size = 10;
    auto result = use_case->execute(filter);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(5, result->items.size());
    EXPECT_EQ(25, result->total_count);
    EXPECT_EQ(3, result->total_pages);
}

TEST_F(TestListItemsUseCase, ListPageBeyondTotalReturnsEmpty)
{
    repo->save(create_item("SKU-001"));

    ItemFilter filter;
    filter.page = 10;
    filter.page_size = 10;
    auto result = use_case->execute(filter);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->items.empty());
    EXPECT_EQ(1, result->total_count);
}

} // namespace inventory::test
