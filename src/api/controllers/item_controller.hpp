#pragma once

#include <drogon/HttpController.h>
#include <nlohmann/json.hpp>
#include <memory>

#include "../../core/use_cases/create_item_use_case.hpp"
#include "../../core/use_cases/delete_item_use_case.hpp"
#include "../../core/use_cases/get_item_use_case.hpp"
#include "../../core/use_cases/list_items_use_case.hpp"
#include "../../core/use_cases/update_item_use_case.hpp"

namespace inventory {

/**
 * @brief HTTP controller for item CRUD operations
 * Path prefix: /api/v1/items
 */
class ItemController : public drogon::HttpController<ItemController> {
public:
    ItemController(
        std::shared_ptr<CreateItemUseCase> create_use_case,
        std::shared_ptr<GetItemUseCase> get_use_case,
        std::shared_ptr<ListItemsUseCase> list_use_case,
        std::shared_ptr<UpdateItemUseCase> update_use_case,
        std::shared_ptr<DeleteItemUseCase> delete_use_case);

    METHOD_LIST_BEGIN
    METHOD_ADD(ItemController::getAllItems, "/", drogon::Get);
    METHOD_ADD(ItemController::createItem, "/", drogon::Post);
    METHOD_ADD(ItemController::getItemById, "/{id}", drogon::Get);
    METHOD_ADD(ItemController::updateItem, "/{id}", drogon::Put);
    METHOD_ADD(ItemController::deleteItem, "/{id}", drogon::Delete);
    METHOD_ADD(ItemController::patchItem, "/{id}", drogon::Patch);
    METHOD_LIST_END

    /**
     * @brief GET /api/v1/items — list items with optional filtering
     */
    void getAllItems(const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /**
     * @brief POST /api/v1/items — create a new item
     */
    void createItem(const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /**
     * @brief GET /api/v1/items/{id} — get item by ID
     */
    void getItemById(const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id);

    /**
     * @brief PUT /api/v1/items/{id} — full update of an item
     */
    void updateItem(const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id);

    /**
     * @brief DELETE /api/v1/items/{id} — delete an item
     */
    void deleteItem(const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id);

    /**
     * @brief PATCH /api/v1/items/{id} — partial update of an item
     */
    void patchItem(const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id);

private:
    std::shared_ptr<CreateItemUseCase> m_create_use_case;
    std::shared_ptr<GetItemUseCase> m_get_use_case;
    std::shared_ptr<ListItemsUseCase> m_list_use_case;
    std::shared_ptr<UpdateItemUseCase> m_update_use_case;
    std::shared_ptr<DeleteItemUseCase> m_delete_use_case;

    static drogon::HttpResponsePtr make_error_response(int status, const std::string& title, const std::string& detail);
    static drogon::HttpResponsePtr make_json_response(const nlohmann::json& data);
};

} // namespace inventory
