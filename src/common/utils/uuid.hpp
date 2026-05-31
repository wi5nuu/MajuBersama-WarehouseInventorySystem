#pragma once

#include <array>
#include <random>
#include <string>
#include <format>

#include <cstdint>

namespace inventory::utils {

class Uuid {
public:
    static Uuid generate()
    {
        static std::mt19937_64 rng(std::random_device{}());
        std::array<uint64_t, 2> data{};
        data[0] = rng();
        data[1] = rng();
        data[0] = (data[0] & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        data[1] = (data[1] & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
        return Uuid(data);
    }

    static Uuid from_string(const std::string& str)
    {
        std::array<uint64_t, 2> data{};
        auto hex = str;
        hex.erase(std::remove(hex.begin(), hex.end(), '-'), hex.end());
        if (hex.size() != 32) {
            return Uuid();
        }
        auto p1 = std::stoull(hex.substr(0, 16), nullptr, 16);
        auto p2 = std::stoull(hex.substr(16, 16), nullptr, 16);
        data[0] = p1;
        data[1] = p2;
        return Uuid(data);
    }

    [[nodiscard]] std::string to_string() const
    {
        auto p1 = m_data[0];
        auto p2 = m_data[1];
        return std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
            (p1 >> 32) & 0xFFFFFFFF,
            (p1 >> 16) & 0xFFFF,
            p1 & 0xFFFF,
            (p2 >> 48) & 0xFFFF,
            p2 & 0xFFFFFFFFFFFFULL);
    }

    [[nodiscard]] bool is_valid() const noexcept
    {
        return m_data[0] != 0 || m_data[1] != 0;
    }

    bool operator==(const Uuid& other) const noexcept = default;

private:
    Uuid() = default;
    explicit Uuid(const std::array<uint64_t, 2>& data)
        : m_data(data)
    {
    }

    std::array<uint64_t, 2> m_data{};
};

} // namespace inventory::utils
