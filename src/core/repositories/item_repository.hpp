#pragma once

#include <optional>
#include <vector>

#include "../entities/item.hpp"
#include "../../common/result.hpp"

namespace inventory {

struct ItemFilter {
    std::optional<std::string> search;
    std::optional<utils::Uuid> category_id;
    std::optional<ItemStatus> status;
    std::optional<int64_t> min_stock;
    std::optional<int64_t> max_price;
    int page{ 1 };
    int page_size{ 20 };
};

class ItemRepository {
public:
    virtual ~ItemRepository() = default;

    virtual Result<Item> find_by_id(const utils::Uuid& id) = 0;
    virtual Result<Item> find_by_sku(const std::string& sku) = 0;
    virtual Result<std::vector<Item>> find_all(const ItemFilter& filter) = 0;
    virtual Result<int64_t> count(const ItemFilter& filter) = 0;
    virtual Result<Item> save(const Item& item) = 0;
    virtual Result<Item> update(const Item& item) = 0;
    virtual Result<void> remove(const utils::Uuid& id) = 0;
    virtual Result<std::vector<Item>> find_low_stock() = 0;
};

} // namespace inventory
