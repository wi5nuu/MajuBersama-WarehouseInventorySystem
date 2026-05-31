#pragma once

#include <memory>
#include <string>

#include "../entities/item.hpp"
#include "../repositories/item_repository.hpp"
#include "../../common/result.hpp"

namespace inventory {

class CreateItemUseCase {
public:
    explicit CreateItemUseCase(std::shared_ptr<ItemRepository> repo)
        : m_repo(std::move(repo))
    {
    }

    struct Request {
        std::string sku;
        std::string name;
        std::string description;
        utils::Uuid category_id;
        utils::Uuid supplier_id;
        int64_t unit_price_amount;
        std::string currency = "IDR";
        int64_t stock_quantity;
        int64_t min_stock_quantity;
        std::string stock_unit = "pcs";
    };

    Result<Item> execute(const Request& req);

private:
    std::shared_ptr<ItemRepository> m_repo;
};

} // namespace inventory
