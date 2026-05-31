#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace inventory {

struct ErrorResponse {
    std::string type{ "about:blank" };
    std::string title{ "Error" };
    int status{ 500 };
    std::string detail;
    std::string instance;
    std::unordered_map<std::string, std::vector<std::string>> errors;

    nlohmann::json to_json() const
    {
        nlohmann::json j = {
            {"type", type},
            {"title", title},
            {"status", status},
            {"detail", detail},
        };
        if (!instance.empty()) {
            j["instance"] = instance;
        }
        if (!errors.empty()) {
            nlohmann::json err_obj;
            for (const auto& [field, msgs] : errors) {
                err_obj[field] = msgs;
            }
            j["errors"] = std::move(err_obj);
        }
        return j;
    }
};

} // namespace inventory
