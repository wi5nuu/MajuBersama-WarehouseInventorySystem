#pragma once

#include <optional>
#include <vector>

#include "../entities/user.hpp"
#include "../../common/result.hpp"

namespace inventory {

class UserRepository {
public:
    virtual ~UserRepository() = default;

    virtual Result<User> find_by_id(const utils::Uuid& id) = 0;
    virtual Result<User> find_by_username(const std::string& username) = 0;
    virtual Result<User> find_by_email(const Email& email) = 0;
    virtual Result<std::vector<User>> find_all(int page = 1, int page_size = 20) = 0;
    virtual Result<User> save(const User& user) = 0;
    virtual Result<User> update(const User& user) = 0;
    virtual Result<void> remove(const utils::Uuid& id) = 0;
};

} // namespace inventory
