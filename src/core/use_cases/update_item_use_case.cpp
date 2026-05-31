#include "update_item_use_case.hpp"

namespace inventory {

Result<Item> UpdateItemUseCase::execute(const Request& req)
{
    auto item_result = m_repo->find_by_id(req.id);
    if (!item_result) {
        return std::unexpected(item_result.error());
    }

    auto item = std::move(item_result.value());

    if (req.name.has_value()) {
        if (req.name.value().empty()) {
            return std::unexpected(Error::validation("Name cannot be empty"));
        }
        item.set_name(req.name.value());
    }

    if (req.description.has_value()) {
        item.set_description(req.description.value());
    }

    if (req.category_id.has_value()) {
        item.set_category_id(req.category_id.value());
    }

    if (req.supplier_id.has_value()) {
        item.set_supplier_id(req.supplier_id.value());
    }

    if (req.unit_price_amount.has_value()) {
        auto price = Money::create(req.unit_price_amount.value(), item.unit_price().currency());
        if (!price) {
            return std::unexpected(price.error());
        }
        item.set_unit_price(price.value());
    }

    if (req.stock_quantity.has_value()) {
        auto qty = Quantity::create(req.stock_quantity.value(), item.stock().unit());
        if (!qty) {
            return std::unexpected(qty.error());
        }
        item.set_stock(qty.value());
    }

    if (req.min_stock_quantity.has_value()) {
        auto qty = Quantity::create(req.min_stock_quantity.value(), item.stock().unit());
        if (!qty) {
            return std::unexpected(qty.error());
        }
        item.set_min_stock(qty.value());
    }

    if (req.status.has_value()) {
        item.set_status(req.status.value());
    }

    item.mark_updated();
    return m_repo->update(item);
}

} // namespace inventory
