#pragma once

#include <memory>

#include "../repositories/item_repository.hpp"
#include "../../common/result.hpp"
#include "../../common/utils/uuid.hpp"

namespace inventory {

class DeleteItemUseCase {
public:
    explicit DeleteItemUseCase(std::shared_ptr<ItemRepository> repo)
        : m_repo(std::move(repo))
    {
    }

    Result<void> execute(const utils::Uuid& id);

private:
    std::shared_ptr<ItemRepository> m_repo;
};

} // namespace inventory
