#include "delete_item_use_case.hpp"

namespace inventory {

Result<void> DeleteItemUseCase::execute(const utils::Uuid& id)
{
    if (!id.is_valid()) {
        return std::unexpected(Error::validation("Invalid item ID"));
    }

    auto exists = m_repo->find_by_id(id);
    if (!exists) {
        return std::unexpected(exists.error());
    }

    return m_repo->remove(id);
}

} // namespace inventory
