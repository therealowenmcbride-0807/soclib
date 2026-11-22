#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace soc {

struct str_view {
    const char *inner = nullptr;
    size_t len = 0;

    constexpr str_view() = default;
    constexpr str_view(const char *ptr, size_t count) : inner(ptr), len(count) {}
    str_view(const char *cstr) : inner(cstr), len(cstr ? std::strlen(cstr) : 0) {}

    constexpr str_view slice(size_t start, size_t count) const {
        if (start >= len) return { nullptr, 0 };
        if (start + count > len) count = len - start;
        return { inner + start, count };
    }

    bool equals(str_view other) const {
        return len == other.len && (len == 0 || std::memcmp(inner, other.inner, len) == 0);
    }

    bool starts_with(str_view prefix) const {
        return len >= prefix.len && (prefix.len == 0 || std::memcmp(inner, prefix.inner, prefix.len) == 0);
    }
};

struct str {
    char *inner = nullptr;
    size_t len = 0;
    size_t cap = 0;

    // Prevent accidental shallow pointer copies
    str() = default;
    str(const str &) = delete;
    str &operator=(const str &) = delete;

    // Safe ownership transfer
    str(str &&other) noexcept;
    str &operator=(str &&other) noexcept;
    ~str();

    // Memory & buffer management (implemented in str.cpp)
    bool grow(size_t min_cap);
    void clear();
    bool append(str_view view);

    // Chained formatting operators (implemented in str.cpp)
    str &operator<<(str_view view);
    str &operator<<(const char *cstr);
    str &operator<<(char ch);
    str &operator<<(int32_t val);
    str &operator<<(uint32_t val);
    str &operator<<(int64_t val);
    str &operator<<(uint64_t val);
    str &operator<<(double val);
    str &operator<<(bool val);

    // Lua-style multi-argument concatenation (must stay in header as a template)
    template <typename... Args>
    str &push(Args &&...args) {
        ((*this << static_cast<Args &&>(args)), ...);
        return *this;
    }

    // Read-only conversions
    str_view view() const { return { inner, len }; }
    const char *cstr() const { return inner ? inner : ""; }
};

} // namespace soc