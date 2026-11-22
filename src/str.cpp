#include <soclib/str.hpp>

#include <array>
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

} // namespace soc