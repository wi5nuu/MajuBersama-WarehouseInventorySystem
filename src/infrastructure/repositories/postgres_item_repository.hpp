#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../core/repositories/item_repository.hpp"
#include "../database/connection_pool.hpp"

namespace inventory {

/**
 * @brief PostgreSQL implementation of the ItemRepository interface.
 *
 * Uses parameterized queries via libpqxx to safely interact with the database.
 */
class PostgresItemRepository : public ItemRepository {
public:
    /**
     * @brief Construct the repository with a connection pool.
     *
     * @param pool Reference to the connection pool.
     */
    explicit PostgresItemRepository(ConnectionPool& pool);

    /**
     * @brief Find an item by its primary key (UUID).
     */
    Result<Item> find_by_id(const utils::Uuid& id) override;

    /**
     * @brief Find an item by its unique SKU.
     */
    Result<Item> find_by_sku(const std::string& sku) override;

    /**
     * @brief Find all items matching the given filter with pagination.
     */
    Result<std::vector<Item>> find_all(const ItemFilter& filter) override;

    /**
     * @brief Count items matching the given filter.
     */
    Result<int64_t> count(const ItemFilter& filter) override;

    /**
     * @brief Insert a new item into the database.
     */
    Result<Item> save(const Item& item) override;

    /**
     * @brief Update an existing item.
     */
    Result<Item> update(const Item& item) override;

    /**
     * @brief Delete an item by its UUID.
     */
    Result<void> remove(const utils::Uuid& id) override;

    /**
     * @brief Find all items whose stock is at or below their minimum stock level.
     */
    Result<std::vector<Item>> find_low_stock() override;

private:
    static std::string status_to_string(ItemStatus s);
    static ItemStatus string_to_status(const std::string& s);
    Item row_to_item(const pqxx::row& row);
    std::pair<std::string, std::vector<std::string>> build_find_all_query(const ItemFilter& filter) const;
    std::pair<std::string, std::vector<std::string>> build_count_query(const ItemFilter& filter) const;

    ConnectionPool& m_pool;
};

} // namespace inventory
