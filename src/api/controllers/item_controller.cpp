#include "item_controller.hpp"

#include <string>
#include <vector>

#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>

#include "../../common/utils/uuid.hpp"
#include "../../core/entities/item.hpp"
#include "../../core/repositories/item_repository.hpp"
#include "../dto/error_response.hpp"
#include "../dto/item_dto.hpp"
#include "../validators/item_validator.hpp"

namespace inventory {

ItemController::ItemController(
    std::shared_ptr<CreateItemUseCase> create_use_case,
    std::shared_ptr<GetItemUseCase> get_use_case,
    std::shared_ptr<ListItemsUseCase> list_use_case,
    std::shared_ptr<UpdateItemUseCase> update_use_case,
    std::shared_ptr<DeleteItemUseCase> delete_use_case)
    : m_create_use_case(std::move(create_use_case))
    , m_get_use_case(std::move(get_use_case))
    , m_list_use_case(std::move(list_use_case))
    , m_update_use_case(std::move(update_use_case))
    , m_delete_use_case(std::move(delete_use_case))
{
}

drogon::HttpResponsePtr ItemController::make_error_response(
    int status, const std::string& title, const std::string& detail)
{
    ErrorResponse err{
        .type = "about:blank",
        .title = title,
        .status = status,
        .detail = detail,
    };
    auto resp = drogon::HttpResponse::newHttpJsonResponse(err.to_json());
    resp->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    return resp;
}

drogon::HttpResponsePtr ItemController::make_json_response(const nlohmann::json& data)
{
    auto resp = drogon::HttpResponse::newHttpJsonResponse(data);
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    return resp;
}

void ItemController::getAllItems(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto params = req->getParameters();

    ItemFilter filter;
    if (params.find("search") != params.end()) {
        filter.search = params["search"];
    }
    if (params.find("category_id") != params.end()) {
        filter.category_id = utils::Uuid::from_string(params["category_id"]);
    }
    if (params.find("status") != params.end()) {
        auto s = params["status"];
        if (s == "active") filter.status = ItemStatus::kActive;
        else if (s == "inactive") filter.status = ItemStatus::kInactive;
        else if (s == "discontinued") filter.status = ItemStatus::kDiscontinued;
        else if (s == "out_of_stock") filter.status = ItemStatus::kOutOfStock;
    }
    if (params.find("page") != params.end()) {
        filter.page = std::stoi(params["page"]);
    }
    if (params.find("page_size") != params.end()) {
        filter.page_size = std::stoi(params["page_size"]);
    }
    if (params.find("min_stock") != params.end()) {
        filter.min_stock = std::stoll(params["min_stock"]);
    }
    if (params.find("max_price") != params.end()) {
        filter.max_price = std::stoll(params["max_price"]);
    }

    auto result = m_list_use_case->execute(filter);
    if (!result) {
        callback(make_error_response(500, "Internal Server Error", result.error().message()));
        return;
    }

    auto& result_set = result.value();
    nlohmann::json items_json = nlohmann::json::array();
    for (const auto& item : result_set.items) {
        items_json.push_back(item_to_response(item));
    }

    nlohmann::json response = {
        {"items", std::move(items_json)},
        {"total_count", result_set.total_count},
        {"page", result_set.page},
        {"page_size", result_set.page_size},
        {"total_pages", result_set.total_pages},
    };

    callback(make_json_response(response));
}

void ItemController::createItem(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto json_ptr = req->getJsonObject();
    if (!json_ptr) {
        callback(make_error_response(400, "Bad Request", "Request body must be valid JSON"));
        return;
    }

    nlohmann::json j = nlohmann::json::parse(json_ptr->toStyledString());
    CreateItemRequest create_req;
    try {
        from_json(j, create_req);
    } catch (const std::exception& e) {
        callback(make_error_response(400, "Bad Request", std::string("Invalid request body: ") + e.what()));
        return;
    }

    auto validation = ItemValidator::validate_create(create_req);
    if (!validation) {
        callback(make_error_response(422, "Validation Error", validation.error().message()));
        return;
    }

    auto result = m_create_use_case->execute({
        .sku = create_req.sku,
        .name = create_req.name,
        .description = create_req.description,
        .category_id = create_req.category_id,
        .supplier_id = create_req.supplier_id,
        .unit_price_amount = create_req.unit_price_amount,
        .currency = create_req.currency,
        .stock_quantity = create_req.stock_quantity,
        .min_stock_quantity = create_req.min_stock_quantity,
        .stock_unit = create_req.stock_unit,
    });

    if (!result) {
        auto& err = result.error();
        int status = 500;
        if (err.code() == ErrorCode::kConflict) {
            status = 409;
        } else if (err.code() == ErrorCode::kValidationError) {
            status = 422;
        }
        callback(make_error_response(status, "Error", err.message()));
        return;
    }

    auto response = item_to_response(result.value());
    auto resp = make_json_response(response);
    resp->setStatusCode(drogon::k201Created);
    callback(resp);
}

void ItemController::getItemById(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    std::string id)
{
    auto uuid = utils::Uuid::from_string(id);
    if (!uuid.is_valid()) {
        callback(make_error_response(400, "Bad Request", "Invalid item ID format"));
        return;
    }

    auto result = m_get_use_case->execute(uuid);
    if (!result) {
        if (result.error().code() == ErrorCode::kNotFound) {
            callback(make_error_response(404, "Not Found", "Item not found"));
        } else {
            callback(make_error_response(500, "Internal Server Error", result.error().message()));
        }
        return;
    }

    auto response = item_to_response(result.value());
    callback(make_json_response(response));
}

void ItemController::updateItem(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    std::string id)
{
    auto uuid = utils::Uuid::from_string(id);
    if (!uuid.is_valid()) {
        callback(make_error_response(400, "Bad Request", "Invalid item ID format"));
        return;
    }

    auto json_ptr = req->getJsonObject();
    if (!json_ptr) {
        callback(make_error_response(400, "Bad Request", "Request body must be valid JSON"));
        return;
    }

    nlohmann::json j = nlohmann::json::parse(json_ptr->toStyledString());
    UpdateItemRequest update_req;
    try {
        from_json(j, update_req);
    } catch (const std::exception& e) {
        callback(make_error_response(400, "Bad Request", std::string("Invalid request body: ") + e.what()));
        return;
    }

    auto validation = ItemValidator::validate_update(update_req);
    if (!validation) {
        callback(make_error_response(422, "Validation Error", validation.error().message()));
        return;
    }

    auto result = m_update_use_case->execute({
        .id = uuid,
        .name = update_req.name,
        .description = update_req.description,
        .category_id = update_req.category_id,
        .supplier_id = update_req.supplier_id,
        .unit_price_amount = update_req.unit_price_amount,
        .stock_quantity = update_req.stock_quantity,
        .min_stock_quantity = update_req.min_stock_quantity,
        .status = update_req.status,
    });

    if (!result) {
        auto& err = result.error();
        int status = 500;
        if (err.code() == ErrorCode::kNotFound) {
            status = 404;
        } else if (err.code() == ErrorCode::kValidationError) {
            status = 422;
        }
        callback(make_error_response(status, "Error", err.message()));
        return;
    }

    auto response = item_to_response(result.value());
    callback(make_json_response(response));
}

void ItemController::deleteItem(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    std::string id)
{
    auto uuid = utils::Uuid::from_string(id);
    if (!uuid.is_valid()) {
        callback(make_error_response(400, "Bad Request", "Invalid item ID format"));
        return;
    }

    auto result = m_delete_use_case->execute(uuid);
    if (!result) {
        auto& err = result.error();
        int status = 500;
        if (err.code() == ErrorCode::kNotFound) {
            status = 404;
        }
        callback(make_error_response(status, "Error", err.message()));
        return;
    }

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k204NoContent);
    callback(resp);
}

void ItemController::patchItem(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    std::string id)
{
    auto uuid = utils::Uuid::from_string(id);
    if (!uuid.is_valid()) {
        callback(make_error_response(400, "Bad Request", "Invalid item ID format"));
        return;
    }

    auto json_ptr = req->getJsonObject();
    if (!json_ptr) {
        callback(make_error_response(400, "Bad Request", "Request body must be valid JSON"));
        return;
    }

    nlohmann::json j = nlohmann::json::parse(json_ptr->toStyledString());
    UpdateItemRequest update_req;
    try {
        from_json(j, update_req);
    } catch (const std::exception& e) {
        callback(make_error_response(400, "Bad Request", std::string("Invalid request body: ") + e.what()));
        return;
    }

    auto validation = ItemValidator::validate_update(update_req);
    if (!validation) {
        callback(make_error_response(422, "Validation Error", validation.error().message()));
        return;
    }

    auto result = m_update_use_case->execute({
        .id = uuid,
        .name = update_req.name,
        .description = update_req.description,
        .category_id = update_req.category_id,
        .supplier_id = update_req.supplier_id,
        .unit_price_amount = update_req.unit_price_amount,
        .stock_quantity = update_req.stock_quantity,
        .min_stock_quantity = update_req.min_stock_quantity,
        .status = update_req.status,
    });

    if (!result) {
        auto& err = result.error();
        int status = 500;
        if (err.code() == ErrorCode::kNotFound) {
            status = 404;
        } else if (err.code() == ErrorCode::kValidationError) {
            status = 422;
        }
        callback(make_error_response(status, "Error", err.message()));
        return;
    }

    auto response = item_to_response(result.value());
    callback(make_json_response(response));
}

} // namespace inventory
