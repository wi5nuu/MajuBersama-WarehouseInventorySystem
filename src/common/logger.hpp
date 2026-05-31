#pragma once

#include <memory>
#include <string>

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace inventory {

class Logger {
public:
    static void init(const std::string& level = "info",
        const std::string& file_path = "/var/log/inventory/app.log",
        std::size_t max_size_mb = 100,
        std::size_t max_files = 7)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            file_path, max_size_mb * 1024 * 1024, max_files);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

        auto logger = std::make_shared<spdlog::logger>(
            "inventory", spdlog::sinks_init_list{ console_sink, file_sink });

        logger->set_level(level_from_string(level));
        logger->flush_on(spdlog::level::err);

        spdlog::set_default_logger(logger);
        spdlog::set_level(level_from_string(level));
    }

    static spdlog::level::level_enum level_from_string(const std::string& level)
    {
        if (level == "debug")
            return spdlog::level::debug;
        if (level == "info")
            return spdlog::level::info;
        if (level == "warn")
            return spdlog::level::warn;
        if (level == "error")
            return spdlog::level::err;
        if (level == "critical")
            return spdlog::level::critical;
        return spdlog::level::info;
    }
};

} // namespace inventory
