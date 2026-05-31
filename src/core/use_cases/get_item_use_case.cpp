#include "get_item_use_case.hpp"

namespace inventory {

Result<Item> GetItemUseCase::execute(const utils::Uuid& id)
{
    if (!id.is_valid()) {
        return std::unexpected(Error::validation("Invalid item ID"));
    }
    return m_repo->find_by_id(id);
}

} // namespace inventory
