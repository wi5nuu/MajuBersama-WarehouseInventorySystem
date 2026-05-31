#include "logging_middleware.hpp"

#include <drogon/HttpResponse.h>

#include "../../common/logger.hpp"

namespace inventory {

void LoggingMiddleware::invoke(
    const drogon::HttpRequestPtr& req,
    drogon::MiddlewareNextCallback&& nextCb,
    drogon::MiddlewareCallback&& mcb)
{
    auto state = std::make_shared<LogState>(
        std::chrono::steady_clock::now(),
        req->methodString(),
        req->path());

    spdlog::info(">>> {} {}", state->method, state->path);

    auto wrapped_mcb = [state, mcb = std::move(mcb)](const drogon::HttpResponsePtr& resp) mutable {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - state->start)
                            .count();
        spdlog::info("<<< {} {} -> {} ({}ms)",
            state->method,
            state->path,
            static_cast<int>(resp->getStatusCode()),
            duration);
        mcb(resp);
    };

    nextCb(std::move(wrapped_mcb));
}

} // namespace inventory
