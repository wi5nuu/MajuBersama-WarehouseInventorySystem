#include "postgres_item_repository.hpp"

#include <chrono>
#include <format>
#include <sstream>

#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

#include "../../common/utils/time_utils.hpp"
#include "../../core/value_objects/money.hpp"
#include "../../core/value_objects/quantity.hpp"

namespace inventory {

PostgresItemRepository::PostgresItemRepository(ConnectionPool& pool)
    : m_pool(pool)
{
}

std::string PostgresItemRepository::status_to_string(ItemStatus s)
{
    switch (s) {
    case ItemStatus::kActive: return "active";
    case ItemStatus::kInactive: return "inactive";
    case ItemStatus::kDiscontinued: return "discontinued";
    case ItemStatus::kOutOfStock: return "out_of_stock";
    }
    return "active";
}

ItemStatus PostgresItemRepository::string_to_status(const std::string& s)
{
    if (s == "active") return ItemStatus::kActive;
    if (s == "inactive") return ItemStatus::kInactive;
    if (s == "discontinued") return ItemStatus::kDiscontinued;
    if (s == "out_of_stock") return ItemStatus::kOutOfStock;
    return ItemStatus::kActive;
}

Result<Item> PostgresItemRepository::find_by_id(const utils::Uuid& id)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "SELECT id, sku, name, description, category_id, supplier_id, "
            "unit_price_amount, unit_price_currency, stock_quantity, stock_unit, "
            "min_stock_quantity, status, created_at, updated_at "
            "FROM items WHERE id = $1",
            id.to_string());

        if (result.empty()) {
            return std::unexpected(Error::not_found(std::format("Item not found: {}", id.to_string())));
        }

        txn.commit();
        return row_to_item(result[0]);
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(std::format("SQL error in find_by_id: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(std::format("Error in find_by_id: {}", ex.what())));
    }
}

Result<Item> PostgresItemRepository::find_by_sku(const std::string& sku)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "SELECT id, sku, name, description, category_id, supplier_id, "
            "unit_price_amount, unit_price_currency, stock_quantity, stock_unit, "
            "min_stock_quantity, status, created_at, updated_at "
            "FROM items WHERE sku = $1",
            sku);

        if (result.empty()) {
            return std::unexpected(Error::not_found(std::format("Item not found by SKU: {}", sku)));
        }

        txn.commit();
        return row_to_item(result[0]);
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(std::format("SQL error in find_by_sku: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(std::format("Error in find_by_sku: {}", ex.what())));
    }
}

Result<std::vector<Item>> PostgresItemRepository::find_all(const ItemFilter& filter)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        auto [query, params] = build_find_all_query(filter);

        pqxx::work txn{ conn_result->get() };

        pqxx::result result;
        if (params.empty()) {
            result = txn.exec(query);
        } else {
            pqxx::params pq_params;
            for (const auto& p : params) {
                pq_params.append(p);
            }
            result = txn.exec_params(query, pq_params);
        }

        std::vector<Item> items;
        items.reserve(result.size());
        for (const auto& row : result) {
            items.emplace_back(row_to_item(row));
        }

        txn.commit();
        return items;
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(std::format("SQL error in find_all: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(std::format("Error in find_all: {}", ex.what())));
    }
}

Result<int64_t> PostgresItemRepository::count(const ItemFilter& filter)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        auto count_filter = filter;
        auto [query, params] = build_count_query(count_filter);

        pqxx::work txn{ conn_result->get() };

        pqxx::result result;
        if (params.empty()) {
            result = txn.exec(query);
        } else {
            pqxx::params pq_params;
            for (const auto& p : params) {
                pq_params.append(p);
            }
            result = txn.exec_params(query, pq_params);
        }

        auto count = result[0][0].as<int64_t>();
        txn.commit();
        return count;
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(std::format("SQL error in count: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(std::format("Error in count: {}", ex.what())));
    }
}

Result<Item> PostgresItemRepository::save(const Item& item)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "INSERT INTO items (id, sku, name, description, category_id, supplier_id, "
            "unit_price_amount, unit_price_currency, stock_quantity, stock_unit, "
            "min_stock_quantity, status, created_at, updated_at) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14) "
            "RETURNING id, sku, name, description, category_id, supplier_id, "
            "unit_price_amount, unit_price_currency, stock_quantity, stock_unit, "
            "min_stock_quantity, status, created_at, updated_at",
            item.id().to_string(),
            item.sku(),
            item.name(),
            item.description(),
            item.category_id().to_string(),
            item.supplier_id().to_string(),
            item.unit_price().amount(),
            item.unit_price().currency(),
            item.stock().value(),
            item.stock().unit(),
            item.min_stock().value(),
            status_to_string(item.status()),
            utils::format_iso8601(item.created_at()),
            utils::format_iso8601(item.updated_at()));

        if (result.empty()) {
            return std::unexpected(Error::database("Failed to insert item"));
        }

        txn.commit();
        return row_to_item(result[0]);
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(std::format("SQL error in save: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(std::format("Error in save: {}", ex.what())));
    }
}

Result<Item> PostgresItemRepository::update(const Item& item)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "UPDATE items SET "
            "sku = $2, name = $3, description = $4, category_id = $5, supplier_id = $6, "
            "unit_price_amount = $7, unit_price_currency = $8, stock_quantity = $9, "
            "stock_unit = $10, min_stock_quantity = $11, status = $12, "
            "updated_at = $13 "
            "WHERE id = $1 "
            "RETURNING id, sku, name, description, category_id, supplier_id, "
            "unit_price_amount, unit_price_currency, stock_quantity, stock_unit, "
            "min_stock_quantity, status, created_at, updated_at",
            item.id().to_string(),
            item.sku(),
            item.name(),
            item.description(),
            item.category_id().to_string(),
            item.supplier_id().to_string(),
            item.unit_price().amount(),
            item.unit_price().currency(),
            item.stock().value(),
            item.stock().unit(),
            item.min_stock().value(),
            status_to_string(item.status()),
            utils::format_iso8601(item.updated_at()));

        if (result.empty()) {
            return std::unexpected(Error::not_found(
                std::format("Item not found for update: {}", item.id().to_string())));
        }

        txn.commit();
        return row_to_item(result[0]);
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(std::format("SQL error in update: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(std::format("Error in update: {}", ex.what())));
    }
}

Result<void> PostgresItemRepository::remove(const utils::Uuid& id)
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec_params(
            "DELETE FROM items WHERE id = $1",
            id.to_string());

        if (result.affected_rows() == 0) {
            return std::unexpected(Error::not_found(
                std::format("Item not found for deletion: {}", id.to_string())));
        }

        txn.commit();
        return {};
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(std::format("SQL error in remove: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(std::format("Error in remove: {}", ex.what())));
    }
}

Result<std::vector<Item>> PostgresItemRepository::find_low_stock()
{
    auto conn_result = m_pool.acquire();
    if (!conn_result) {
        return std::unexpected(Error::database(
            std::format("Failed to acquire connection: {}", conn_result.error().message())));
    }

    try {
        pqxx::work txn{ conn_result->get() };
        auto result = txn.exec(
            "SELECT id, sku, name, description, category_id, supplier_id, "
            "unit_price_amount, unit_price_currency, stock_quantity, stock_unit, "
            "min_stock_quantity, status, created_at, updated_at "
            "FROM items WHERE stock_quantity <= min_stock_quantity AND status = 'active' "
            "ORDER BY stock_quantity ASC");

        std::vector<Item> items;
        items.reserve(result.size());
        for (const auto& row : result) {
            items.emplace_back(row_to_item(row));
        }

        txn.commit();
        return items;
    } catch (const pqxx::sql_error& ex) {
        return std::unexpected(Error::database(std::format("SQL error in find_low_stock: {}", ex.what())));
    } catch (const std::exception& ex) {
        return std::unexpected(Error::internal(std::format("Error in find_low_stock: {}", ex.what())));
    }
}

Item PostgresItemRepository::row_to_item(const pqxx::row& row)
{
    auto unit_price = Money::create(
        row["unit_price_amount"].as<int64_t>(),
        row["unit_price_currency"].as<std::string>("IDR"))
                          .value_or(Money::create(0).value());

    auto stock = Quantity::create(
        row["stock_quantity"].as<int64_t>(),
        row["stock_unit"].as<std::string>("pcs"))
                     .value_or(Quantity::create(0).value());

    auto min_stock = Quantity::create(
        row["min_stock_quantity"].as<int64_t>(),
        row["stock_unit"].as<std::string>("pcs"))
                         .value_or(Quantity::create(0).value());

    auto status_str = row["status"].as<std::string>("active");
    auto cat_id_str = row["category_id"].as<std::string>("");
    auto sup_id_str = row["supplier_id"].as<std::string>("");

    return Item(
        utils::Uuid::from_string(row["id"].as<std::string>()),
        row["sku"].as<std::string>(),
        row["name"].as<std::string>(),
        row["description"].as<std::string>(""),
        cat_id_str.empty() ? utils::Uuid{} : utils::Uuid::from_string(cat_id_str),
        sup_id_str.empty() ? utils::Uuid{} : utils::Uuid::from_string(sup_id_str),
        unit_price,
        stock,
        min_stock,
        string_to_status(status_str),
        utils::parse_iso8601(row["created_at"].as<std::string>()),
        utils::parse_iso8601(row["updated_at"].as<std::string>()));
}

std::pair<std::string, std::vector<std::string>> PostgresItemRepository::build_find_all_query(
    const ItemFilter& filter) const
{
    std::vector<std::string> params;
    std::ostringstream sql;

    sql << "SELECT id, sku, name, description, category_id, supplier_id, "
        << "unit_price_amount, unit_price_currency, stock_quantity, stock_unit, "
        << "min_stock_quantity, status, created_at, updated_at FROM items WHERE 1=1";

    auto add_param = [&](const std::string& clause, const std::string& value) {
        params.push_back(value);
        sql << clause << "$" << params.size();
    };

    if (filter.search.has_value() && !filter.search->empty()) {
        auto search_pattern = std::format("%{}%", *filter.search);
        sql << " AND (";
        params.push_back(search_pattern);
        sql << "name ILIKE $" << params.size();
        sql << " OR ";
        params.push_back(search_pattern);
        sql << "sku ILIKE $" << params.size();
        sql << ")";
    }

    if (filter.category_id.has_value() && filter.category_id->is_valid()) {
        add_param(" AND category_id = ", filter.category_id->to_string());
    }

    if (filter.status.has_value()) {
        add_param(" AND status = ", status_to_string(*filter.status));
    }

    if (filter.min_stock.has_value()) {
        add_param(" AND stock_quantity <= ", std::to_string(*filter.min_stock));
    }

    if (filter.max_price.has_value()) {
        add_param(" AND unit_price_amount <= ", std::to_string(*filter.max_price));
    }

    sql << " ORDER BY created_at DESC";

    auto offset = (filter.page - 1) * filter.page_size;
    params.push_back(std::to_string(filter.page_size));
    sql << " LIMIT $" << params.size();
    params.push_back(std::to_string(offset));
    sql << " OFFSET $" << params.size();

    return { sql.str(), std::move(params) };
}

std::pair<std::string, std::vector<std::string>> PostgresItemRepository::build_count_query(
    const ItemFilter& filter) const
{
    std::vector<std::string> params;
    std::ostringstream sql;

    sql << "SELECT COUNT(*) FROM items WHERE 1=1";

    auto add_param = [&](const std::string& clause, const std::string& value) {
        params.push_back(value);
        sql << clause << "$" << params.size();
    };

    if (filter.search.has_value() && !filter.search->empty()) {
        auto search_pattern = std::format("%{}%", *filter.search);
        sql << " AND (";
        params.push_back(search_pattern);
        sql << "name ILIKE $" << params.size();
        sql << " OR ";
        params.push_back(search_pattern);
        sql << "sku ILIKE $" << params.size();
        sql << ")";
    }

    if (filter.category_id.has_value() && filter.category_id->is_valid()) {
        add_param(" AND category_id = ", filter.category_id->to_string());
    }

    if (filter.status.has_value()) {
        add_param(" AND status = ", status_to_string(*filter.status));
    }

    if (filter.min_stock.has_value()) {
        add_param(" AND stock_quantity <= ", std::to_string(*filter.min_stock));
    }

    if (filter.max_price.has_value()) {
        add_param(" AND unit_price_amount <= ", std::to_string(*filter.max_price));
    }

    return { sql.str(), std::move(params) };
}

} // namespace inventory
