#include "item_validator.hpp"

namespace inventory {

Result<void> ItemValidator::validate_create(const CreateItemRequest& request)
{
    if (request.sku.empty()) {
        return std::unexpected(Error::validation("SKU cannot be empty"));
    }

    if (request.name.empty()) {
        return std::unexpected(Error::validation("Name cannot be empty"));
    }

    if (request.unit_price_amount < 0) {
        return std::unexpected(Error::validation("Price cannot be negative"));
    }

    if (request.stock_quantity < 0) {
        return std::unexpected(Error::validation("Stock quantity cannot be negative"));
    }

    if (request.min_stock_quantity < 0) {
        return std::unexpected(Error::validation("Minimum stock quantity cannot be negative"));
    }

    if (request.currency.size() != 3) {
        return std::unexpected(Error::validation("Currency must be a 3-letter ISO code"));
    }

    if (request.stock_unit.empty()) {
        return std::unexpected(Error::validation("Stock unit cannot be empty"));
    }

    return {};
}

Result<void> ItemValidator::validate_update(const UpdateItemRequest& request)
{
    if (request.sku.empty()) {
        return std::unexpected(Error::validation("SKU cannot be empty"));
    }

    if (request.unit_price_amount.has_value() && request.unit_price_amount.value() < 0) {
        return std::unexpected(Error::validation("Price cannot be negative"));
    }

    if (request.stock_quantity.has_value() && request.stock_quantity.value() < 0) {
        return std::unexpected(Error::validation("Stock quantity cannot be negative"));
    }

    if (request.min_stock_quantity.has_value() && request.min_stock_quantity.value() < 0) {
        return std::unexpected(Error::validation("Minimum stock quantity cannot be negative"));
    }

    return {};
}

} // namespace inventory
