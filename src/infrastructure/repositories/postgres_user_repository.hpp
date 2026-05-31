#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../core/repositories/user_repository.hpp"
#include "../database/connection_pool.hpp"

namespace inventory {

/**
 * @brief PostgreSQL implementation of the UserRepository interface.
 *
 * Uses parameterized queries via libpqxx for safe database interaction.
 */
class PostgresUserRepository : public UserRepository {
public:
    /**
     * @brief Construct the repository with a connection pool.
     *
     * @param pool Reference to the connection pool.
     */
    explicit PostgresUserRepository(ConnectionPool& pool);

    /**
     * @brief Find a user by their primary key (UUID).
     */
    Result<User> find_by_id(const utils::Uuid& id) override;

    /**
     * @brief Find a user by their unique username.
     */
    Result<User> find_by_username(const std::string& username) override;

    /**
     * @brief Find a user by their email address.
     */
    Result<User> find_by_email(const Email& email) override;

    /**
     * @brief Find all users with pagination.
     */
    Result<std::vector<User>> find_all(int page = 1, int page_size = 20) override;

    /**
     * @brief Insert a new user into the database.
     */
    Result<User> save(const User& user) override;

    /**
     * @brief Update an existing user.
     */
    Result<User> update(const User& user) override;

    /**
     * @brief Delete a user by their UUID.
     */
    Result<void> remove(const utils::Uuid& id) override;

private:
    User row_to_user(const pqxx::row& row);

    ConnectionPool& m_pool;
};

} // namespace inventory
