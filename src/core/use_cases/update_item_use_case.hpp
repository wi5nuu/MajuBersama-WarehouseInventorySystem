#pragma once

#include <memory>

#include "../entities/item.hpp"
#include "../repositories/item_repository.hpp"
#include "../../common/result.hpp"
#include "../../common/utils/uuid.hpp"

namespace inventory {

class UpdateItemUseCase {
public:
    explicit UpdateItemUseCase(std::shared_ptr<ItemRepository> repo)
        : m_repo(std::move(repo))
    {
    }

    struct Request {
        utils::Uuid id;
        std::optional<std::string> name;
        std::optional<std::string> description;
        std::optional<utils::Uuid> category_id;
        std::optional<utils::Uuid> supplier_id;
        std::optional<int64_t> unit_price_amount;
        std::optional<int64_t> stock_quantity;
        std::optional<int64_t> min_stock_quantity;
        std::optional<ItemStatus> status;
    };

    Result<Item> execute(const Request& req);

private:
    std::shared_ptr<ItemRepository> m_repo;
};

} // namespace inventory
