#pragma once

#include <memory>

#include "../entities/item.hpp"
#include "../repositories/item_repository.hpp"
#include "../../common/result.hpp"
#include "../../common/utils/uuid.hpp"

namespace inventory {

class GetItemUseCase {
public:
    explicit GetItemUseCase(std::shared_ptr<ItemRepository> repo)
        : m_repo(std::move(repo))
    {
    }

    Result<Item> execute(const utils::Uuid& id);

private:
    std::shared_ptr<ItemRepository> m_repo;
};

} // namespace inventory
