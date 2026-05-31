#include "create_item_use_case.hpp"

#include <chrono>

namespace inventory {

Result<Item> CreateItemUseCase::execute(const Request& req)
{
    auto price_result = Money::create(req.unit_price_amount, req.currency);
    if (!price_result) {
        return std::unexpected(price_result.error());
    }

    auto stock_result = Quantity::create(req.stock_quantity, req.stock_unit);
    if (!stock_result) {
        return std::unexpected(stock_result.error());
    }

    auto min_stock_result = Quantity::create(req.min_stock_quantity, req.stock_unit);
    if (!min_stock_result) {
        return std::unexpected(min_stock_result.error());
    }

    auto existing = m_repo->find_by_sku(req.sku);
    if (existing) {
        return std::unexpected(Error::conflict("Item with SKU '" + req.sku + "' already exists"));
    }

    auto now = std::chrono::system_clock::now();
    Item item(
        utils::Uuid::generate(),
        req.sku,
        req.name,
        req.description,
        req.category_id,
        req.supplier_id,
        price_result.value(),
        stock_result.value(),
        min_stock_result.value(),
        ItemStatus::kActive,
        now,
        now);

    return m_repo->save(item);
}

} // namespace inventory
