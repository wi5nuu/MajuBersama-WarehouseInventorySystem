#pragma once

#include <vector>

#include "../entities/item.hpp"
#include "../repositories/item_repository.hpp"
#include "../../common/result.hpp"

namespace inventory {

class InventoryService {
public:
    virtual ~InventoryService() = default;

    virtual Result<Item> add_stock(const utils::Uuid& item_id, const Quantity& quantity) = 0;
    virtual Result<Item> remove_stock(const utils::Uuid& item_id, const Quantity& quantity) = 0;
    virtual Result<std::vector<Item>> get_low_stock_items() = 0;
    virtual Result<void> transfer_stock(const utils::Uuid& from_warehouse, const utils::Uuid& to_warehouse, const utils::Uuid& item_id, const Quantity& quantity) = 0;
    virtual Result<Quantity> get_available_stock(const utils::Uuid& item_id) = 0;
};

} // namespace inventory
