#pragma once

#include <chrono>
#include <format>
#include <string>
#include <string_view>

namespace inventory::utils {

using TimePoint = std::chrono::system_clock::time_point;

[[nodiscard]] inline TimePoint now() noexcept
{
    return std::chrono::system_clock::now();
}

[[nodiscard]] inline std::string format_iso8601(TimePoint tp)
{
    auto tt = std::chrono::system_clock::to_time_t(tp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  tp.time_since_epoch())
                  .count()
        % 1000;
    std::tm tm{};
    gmtime_s(&tm, &tt);
    return std::format("{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}.{:03d}Z",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec, static_cast<int>(ms));
}

[[nodiscard]] inline TimePoint parse_iso8601(std::string_view sv)
{
    std::tm tm{};
    std::istringstream ss{ std::string(sv) };
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    auto tp = std::chrono::system_clock::from_time_t(timegm(&tm));
    auto dot = sv.find('.');
    if (dot != std::string_view::npos) {
        auto ms_str = sv.substr(dot + 1, 3);
        auto ms = std::stoi(std::string(ms_str));
        tp += std::chrono::milliseconds(ms);
    }
    return tp;
}

} // namespace inventory::utils
