#pragma once

#include <drogon/HttpMiddleware.h>

#include <chrono>
#include <string>

namespace inventory {

/**
 * @brief Middleware that logs HTTP request method, path, response status, and duration
 */
class LoggingMiddleware : public drogon::HttpMiddleware<LoggingMiddleware> {
public:
    void invoke(const drogon::HttpRequestPtr& req,
        drogon::MiddlewareNextCallback&& nextCb,
        drogon::MiddlewareCallback&& mcb) override;

private:
    struct LogState {
        std::chrono::steady_clock::time_point start;
        std::string method;
        std::string path;
    };
};

} // namespace inventory
