#pragma once

#include <optional>
#include <vector>

#include "../entities/warehouse.hpp"
#include "../../common/result.hpp"

namespace inventory {

class WarehouseRepository {
public:
    virtual ~WarehouseRepository() = default;

    virtual Result<Warehouse> find_by_id(const utils::Uuid& id) = 0;
    virtual Result<Warehouse> find_by_code(const std::string& code) = 0;
    virtual Result<std::vector<Warehouse>> find_all(bool active_only = true) = 0;
    virtual Result<Warehouse> save(const Warehouse& warehouse) = 0;
    virtual Result<Warehouse> update(const Warehouse& warehouse) = 0;
    virtual Result<void> remove(const utils::Uuid& id) = 0;
};

} // namespace inventory
