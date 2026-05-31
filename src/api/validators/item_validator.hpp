#pragma once

#include <string>

#include "../../common/result.hpp"
#include "../dto/item_dto.hpp"

namespace inventory {

class ItemValidator {
public:
    /**
     * @brief Validates a create item request
     * @param request The create item request to validate
     * @return Result<void> containing validation errors if any
     */
    static Result<void> validate_create(const CreateItemRequest& request);

    /**
     * @brief Validates an update item request
     * @param request The update item request to validate
     * @return Result<void> containing validation errors if any
     */
    static Result<void> validate_update(const UpdateItemRequest& request);
};

} // namespace inventory
