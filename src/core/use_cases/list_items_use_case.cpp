#include "list_items_use_case.hpp"

#include <algorithm>
#include <cmath>

namespace inventory {

Result<ListItemsUseCase::ResultSet> ListItemsUseCase::execute(const ItemFilter& filter)
{
    auto items = m_repo->find_all(filter);
    if (!items) {
        return std::unexpected(items.error());
    }

    auto count = m_repo->count(filter);
    if (!count) {
        return std::unexpected(count.error());
    }

    int total_pages = static_cast<int>(std::ceil(static_cast<double>(count.value()) / filter.page_size));

    return ResultSet{
        std::move(items.value()),
        count.value(),
        filter.page,
        filter.page_size,
        total_pages
    };
}

} // namespace inventory
