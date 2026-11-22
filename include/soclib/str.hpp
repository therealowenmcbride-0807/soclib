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

namespace fmt {

struct Hex {
    uint64_t value;
    bool prefix;
    bool uppercase;
};

struct Bin {
    uint64_t value;
    bool prefix;
};

struct Upper {
    str_view text;
};

struct Lower {
    str_view text;
};

struct Pad {
    str_view text;
    size_t width;
    char fill;
    bool align_right;
};

// Ergonomic helper constructors
constexpr Hex hex(uint64_t v, bool prefix = true, bool uppercase = false) {
    return { v, prefix, uppercase };
}

constexpr Bin bin(uint64_t v, bool prefix = true) {
    return { v, prefix };
}

inline Upper upper(str_view text) { return { text }; }
inline Lower lower(str_view text) { return { text }; }

inline Pad pad_left(str_view text, size_t width, char fill = ' ') {
    return { text, width, fill, true };
}

inline Pad pad_right(str_view text, size_t width, char fill = ' ') {
    return { text, width, fill, false };
}

} // namespace fmt

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

    // Memory & buffer management
    bool grow(size_t min_cap);
    void clear();
    bool append(str_view view);

    // Chained formatting operators
    str &operator<<(str_view view);
    str &operator<<(const char *cstr);
    str &operator<<(char ch);
    str &operator<<(int32_t val);
    str &operator<<(uint32_t val);
    str &operator<<(int64_t val);
    str &operator<<(uint64_t val);
    str &operator<<(double val);
    str &operator<<(bool val);
    str &operator<<(fmt::Hex val);
    str &operator<<(fmt::Bin val);
    str &operator<<(fmt::Upper val);
    str &operator<<(fmt::Lower val);
    str &operator<<(fmt::Pad val);

    // Lua-style multi-argument concatenation
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