#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "../../common/utils/uuid.hpp"
#include "../../core/entities/item.hpp"

namespace inventory {

struct CreateItemRequest {
    std::string sku;
    std::string name;
    std::string description;
    utils::Uuid category_id;
    utils::Uuid supplier_id;
    int64_t unit_price_amount{ 0 };
    std::string currency{ "IDR" };
    int64_t stock_quantity{ 0 };
    int64_t min_stock_quantity{ 0 };
    std::string stock_unit{ "pcs" };
};

struct UpdateItemRequest {
    std::string sku;
    std::optional<std::string> name;
    std::optional<std::string> description;
    std::optional<utils::Uuid> category_id;
    std::optional<utils::Uuid> supplier_id;
    std::optional<int64_t> unit_price_amount;
    std::optional<std::string> currency;
    std::optional<int64_t> stock_quantity;
    std::optional<int64_t> min_stock_quantity;
    std::optional<std::string> stock_unit;
    std::optional<ItemStatus> status;
};

struct ItemResponse {
    std::string id;
    std::string sku;
    std::string name;
    std::string description;
    std::string category_id;
    std::string supplier_id;
    int64_t unit_price_amount;
    std::string currency;
    int64_t stock_quantity;
    std::string stock_unit;
    int64_t min_stock_quantity;
    std::string status;
    std::string created_at;
    std::string updated_at;
};

inline void to_json(nlohmann::json& j, const CreateItemRequest& req)
{
    j = nlohmann::json{
        {"sku", req.sku},
        {"name", req.name},
        {"description", req.description},
        {"category_id", req.category_id.to_string()},
        {"supplier_id", req.supplier_id.to_string()},
        {"unit_price_amount", req.unit_price_amount},
        {"currency", req.currency},
        {"stock_quantity", req.stock_quantity},
        {"min_stock_quantity", req.min_stock_quantity},
        {"stock_unit", req.stock_unit},
    };
}

inline void from_json(const nlohmann::json& j, CreateItemRequest& req)
{
    j.at("sku").get_to(req.sku);
    j.at("name").get_to(req.name);
    j.at("description").get_to(req.description);
    req.category_id = utils::Uuid::from_string(j.at("category_id").get<std::string>());
    req.supplier_id = utils::Uuid::from_string(j.at("supplier_id").get<std::string>());
    j.at("unit_price_amount").get_to(req.unit_price_amount);
    if (j.contains("currency")) {
        j.at("currency").get_to(req.currency);
    }
    j.at("stock_quantity").get_to(req.stock_quantity);
    j.at("min_stock_quantity").get_to(req.min_stock_quantity);
    if (j.contains("stock_unit")) {
        j.at("stock_unit").get_to(req.stock_unit);
    }
}

inline void to_json(nlohmann::json& j, const UpdateItemRequest& req)
{
    j = nlohmann::json{
        {"sku", req.sku},
    };
    if (req.name.has_value()) {
        j["name"] = req.name.value();
    }
    if (req.description.has_value()) {
        j["description"] = req.description.value();
    }
    if (req.category_id.has_value()) {
        j["category_id"] = req.category_id.value().to_string();
    }
    if (req.supplier_id.has_value()) {
        j["supplier_id"] = req.supplier_id.value().to_string();
    }
    if (req.unit_price_amount.has_value()) {
        j["unit_price_amount"] = req.unit_price_amount.value();
    }
    if (req.currency.has_value()) {
        j["currency"] = req.currency.value();
    }
    if (req.stock_quantity.has_value()) {
        j["stock_quantity"] = req.stock_quantity.value();
    }
    if (req.min_stock_quantity.has_value()) {
        j["min_stock_quantity"] = req.min_stock_quantity.value();
    }
    if (req.stock_unit.has_value()) {
        j["stock_unit"] = req.stock_unit.value();
    }
    if (req.status.has_value()) {
        switch (req.status.value()) {
        case ItemStatus::kActive: j["status"] = "active"; break;
        case ItemStatus::kInactive: j["status"] = "inactive"; break;
        case ItemStatus::kDiscontinued: j["status"] = "discontinued"; break;
        case ItemStatus::kOutOfStock: j["status"] = "out_of_stock"; break;
        }
    }
}

inline void from_json(const nlohmann::json& j, UpdateItemRequest& req)
{
    j.at("sku").get_to(req.sku);
    if (j.contains("name")) {
        req.name = j.at("name").get<std::string>();
    }
    if (j.contains("description")) {
        req.description = j.at("description").get<std::string>();
    }
    if (j.contains("category_id")) {
        req.category_id = utils::Uuid::from_string(j.at("category_id").get<std::string>());
    }
    if (j.contains("supplier_id")) {
        req.supplier_id = utils::Uuid::from_string(j.at("supplier_id").get<std::string>());
    }
    if (j.contains("unit_price_amount")) {
        req.unit_price_amount = j.at("unit_price_amount").get<int64_t>();
    }
    if (j.contains("currency")) {
        req.currency = j.at("currency").get<std::string>();
    }
    if (j.contains("stock_quantity")) {
        req.stock_quantity = j.at("stock_quantity").get<int64_t>();
    }
    if (j.contains("min_stock_quantity")) {
        req.min_stock_quantity = j.at("min_stock_quantity").get<int64_t>();
    }
    if (j.contains("stock_unit")) {
        req.stock_unit = j.at("stock_unit").get<std::string>();
    }
}

inline ItemResponse item_to_response(const Item& item)
{
    auto t_created = std::chrono::system_clock::to_time_t(item.created_at());
    auto t_updated = std::chrono::system_clock::to_time_t(item.updated_at());
    std::string created_str = std::ctime(&t_created);
    std::string updated_str = std::ctime(&t_updated);
    if (!created_str.empty()) created_str.pop_back();
    if (!updated_str.empty()) updated_str.pop_back();

    return ItemResponse{
        item.id().to_string(),
        item.sku(),
        item.name(),
        item.description(),
        item.category_id().to_string(),
        item.supplier_id().to_string(),
        item.unit_price().amount(),
        item.unit_price().currency(),
        item.stock().value(),
        item.stock().unit(),
        item.min_stock().value(),
        [](ItemStatus s) -> std::string {
            switch (s) {
            case ItemStatus::kActive: return "active";
            case ItemStatus::kInactive: return "inactive";
            case ItemStatus::kDiscontinued: return "discontinued";
            case ItemStatus::kOutOfStock: return "out_of_stock";
            }
            return "unknown";
        }(item.status()),
        created_str,
        updated_str
    };
}

inline void to_json(nlohmann::json& j, const ItemResponse& resp)
{
    j = nlohmann::json{
        {"id", resp.id},
        {"sku", resp.sku},
        {"name", resp.name},
        {"description", resp.description},
        {"category_id", resp.category_id},
        {"supplier_id", resp.supplier_id},
        {"unit_price_amount", resp.unit_price_amount},
        {"currency", resp.currency},
        {"stock_quantity", resp.stock_quantity},
        {"stock_unit", resp.stock_unit},
        {"min_stock_quantity", resp.min_stock_quantity},
        {"status", resp.status},
        {"created_at", resp.created_at},
        {"updated_at", resp.updated_at},
    };
}

} // namespace inventory
