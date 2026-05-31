#pragma once

#include <memory>
#include <vector>

#include "../entities/item.hpp"
#include "../repositories/item_repository.hpp"
#include "../../common/result.hpp"

namespace inventory {

class ListItemsUseCase {
public:
    explicit ListItemsUseCase(std::shared_ptr<ItemRepository> repo)
        : m_repo(std::move(repo))
    {
    }

    struct ResultSet {
        std::vector<Item> items;
        int64_t total_count;
        int page;
        int page_size;
        int total_pages;
    };

    Result<ResultSet> execute(const ItemFilter& filter);

private:
    std::shared_ptr<ItemRepository> m_repo;
};

} // namespace inventory
