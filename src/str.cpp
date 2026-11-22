#include <soclib/str.hpp>

#include <array>
#include <cctype>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>

namespace soc {

str::str(str &&other) noexcept
    : inner(other.inner), len(other.len), cap(other.cap) {
    other.inner = nullptr;
    other.len = 0;
    other.cap = 0;
}

str &str::operator=(str &&other) noexcept {
    if (this != &other) {
        std::free(inner);
        inner = other.inner;
        len = other.len;
        cap = other.cap;
        other.inner = nullptr;
        other.len = 0;
        other.cap = 0;
    }
    return *this;
}

str::~str() {
    std::free(inner);
}

bool str::grow(size_t min_cap) {
    if (min_cap <= cap) return true;

    size_t next_cap = (cap == 0) ? 16 : cap * 2;
    if (next_cap < min_cap) next_cap = min_cap;

    // +1 guarantees space for the trailing '\0'
    char *next = static_cast<char *>(std::realloc(inner, next_cap + 1));
    if (!next) return false;

    inner = next;
    cap = next_cap;
    return true;
}

void str::clear() {
    len = 0;
    if (inner) *inner = '\0';
}

bool str::append(str_view view) {
    if (view.len == 0) return true;
    if (!grow(len + view.len)) return false;

    std::memcpy(inner + len, view.inner, view.len);
    len += view.len;
    inner[len] = '\0';
    return true;
}

str &str::operator<<(str_view view) {
    append(view);
    return *this;
}

str &str::operator<<(const char *cstr) {
    append(str_view(cstr));
    return *this;
}

str &str::operator<<(char ch) {
    append({ &ch, 1 });
    return *this;
}

str &str::operator<<(int32_t val) {
    std::array<char, 32> buf;
    int n = std::snprintf(buf.data(), buf.size(), "%" PRId32, val);
    if (n > 0) append({ buf.data(), static_cast<size_t>(n) });
    return *this;
}

str &str::operator<<(uint32_t val) {
    std::array<char, 32> buf;
    int n = std::snprintf(buf.data(), buf.size(), "%" PRIu32, val);
    if (n > 0) append({ buf.data(), static_cast<size_t>(n) });
    return *this;
}

str &str::operator<<(int64_t val) {
    std::array<char, 32> buf;
    int n = std::snprintf(buf.data(), buf.size(), "%" PRId64, val);
    if (n > 0) append({ buf.data(), static_cast<size_t>(n) });
    return *this;
}

str &str::operator<<(uint64_t val) {
    std::array<char, 32> buf;
    int n = std::snprintf(buf.data(), buf.size(), "%" PRIu64, val);
    if (n > 0) append({ buf.data(), static_cast<size_t>(n) });
    return *this;
}

str &str::operator<<(double val) {
    std::array<char, 64> buf;
    int n = std::snprintf(buf.data(), buf.size(), "%.6g", val);
    if (n > 0) append({ buf.data(), static_cast<size_t>(n) });
    return *this;
}

str &str::operator<<(bool val) {
    append(val ? str_view("true", 4) : str_view("false", 5));
    return *this;
}

str &str::operator<<(fmt::Hex h) {
    std::array<char, 32> buf;
    const char *spec = h.uppercase
        ? (h.prefix ? "0X%" PRIX64 : "%" PRIX64)
        : (h.prefix ? "0x%" PRIx64 : "%" PRIx64);
    int n = std::snprintf(buf.data(), buf.size(), spec, h.value);
    if (n > 0) append({ buf.data(), static_cast<size_t>(n) });
    return *this;
}

str &str::operator<<(fmt::Bin b) {
    if (b.prefix) append(str_view("0b", 2));
    if (b.value == 0) {
        append(str_view("0", 1));
        return *this;
    }

    std::array<char, 64> buf;
    size_t pos = buf.size();
    uint64_t v = b.value;
    while (v > 0 && pos > 0) {
        --pos;
        *(buf.data() + pos) = (v & 1ULL) ? '1' : '0';
        v >>= 1;
    }
    append({ buf.data() + pos, buf.size() - pos });
    return *this;
}

str &str::operator<<(fmt::Upper u) {
    if (u.text.len == 0) return *this;
    if (!grow(len + u.text.len)) return *this;

    for (size_t i = 0; i < u.text.len; ++i) {
        unsigned char ch = static_cast<unsigned char>(*(u.text.inner + i));
        *(inner + len + i) = static_cast<char>(std::toupper(ch));
    }
    len += u.text.len;
    *(inner + len) = '\0';
    return *this;
}

str &str::operator<<(fmt::Lower l) {
    if (l.text.len == 0) return *this;
    if (!grow(len + l.text.len)) return *this;

    for (size_t i = 0; i < l.text.len; ++i) {
        unsigned char ch = static_cast<unsigned char>(*(l.text.inner + i));
        *(inner + len + i) = static_cast<char>(std::tolower(ch));
    }
    len += l.text.len;
    *(inner + len) = '\0';
    return *this;
}

str &str::operator<<(fmt::Pad p) {
    if (p.text.len >= p.width) {
        append(p.text);
        return *this;
    }
    size_t pad_count = p.width - p.text.len;
    if (!grow(len + p.width)) return *this;

    if (p.align_right) {
        std::memset(inner + len, p.fill, pad_count);
        len += pad_count;
        append(p.text);
    } else {
        append(p.text);
        std::memset(inner + len, p.fill, pad_count);
        len += pad_count;
        *(inner + len) = '\0';
    }
    return *this;
}

} // namespace soc