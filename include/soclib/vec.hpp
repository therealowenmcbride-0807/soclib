#pragma once

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <new>
#include <type_traits>

namespace soc {

template <typename T>
struct Slice {
    T *inner = nullptr;
    size_t len = 0;

    constexpr Slice() = default;
    constexpr Slice(T *ptr, size_t count) : inner(ptr), len(count) {}

    T &at(size_t index) const {
        assert(index < len);
        return *(inner + index);
    }

    constexpr Slice subslice(size_t start, size_t count) const {
        if (start >= len) return { nullptr, 0 };
        if (start + count > len) count = len - start;
        return { inner + start, count };
    }

    constexpr T *begin() const { return inner; }
    constexpr T *end() const { return inner + len; }
};

template <typename T>
struct Vec {
    T *inner = nullptr;
    size_t len = 0;
    size_t cap = 0;

    Vec() = default;
    Vec(const Vec &) = delete;
    Vec &operator=(const Vec &) = delete;

    Vec(Vec &&other) noexcept : inner(other.inner), len(other.len), cap(other.cap) {
        other.inner = nullptr;
        other.len = 0;
        other.cap = 0;
    }

    Vec &operator=(Vec &&other) noexcept {
        if (this != &other) {
            clear();
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

    ~Vec() {
        clear();
        std::free(inner);
    }

    bool grow(size_t min_cap) {
        if (min_cap <= cap) return true;

        size_t next_cap = (cap == 0) ? 8 : cap * 2;
        if (next_cap < min_cap) next_cap = min_cap;

        if constexpr (std::is_trivially_copyable_v<T>) {
            // Fast path: raw C realloc for plain structs, ints, floats, pointers
            T *next = static_cast<T *>(std::realloc(inner, next_cap * sizeof(T)));
            if (!next) return false;
            inner = next;
        } else {
            // Safe path: move non-trivial structs (e.g., Vec<soc::str>)
            T *next = static_cast<T *>(std::malloc(next_cap * sizeof(T)));
            if (!next) return false;
            for (size_t i = 0; i < len; ++i) {
                new (next + i) T(static_cast<T &&>(*(inner + i)));
                (inner + i)->~T();
            }
            std::free(inner);
            inner = next;
        }

        cap = next_cap;
        return true;
    }

    bool push(T item) {
        if (!grow(len + 1)) return false;
        new (inner + len) T(static_cast<T &&>(item));
        ++len;
        return true;
    }

    void pop() {
        assert(len > 0);
        --len;
        if constexpr (!std::is_trivially_destructible_v<T>) {
            (inner + len)->~T();
        }
    }

    void clear() {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0; i < len; ++i) {
                (inner + i)->~T();
            }
        }
        len = 0;
    }

    T &at(size_t index) {
        assert(index < len);
        return *(inner + index);
    }

    const T &at(size_t index) const {
        assert(index < len);
        return *(inner + index);
    }

    T &back() {
        assert(len > 0);
        return *(inner + len - 1);
    }

    Slice<T> slice() { return { inner, len }; }
    Slice<const T> slice() const { return { inner, len }; }

    // Enables clean range-based loops: for (auto &x : vec)
    T *begin() { return inner; }
    T *end() { return inner + len; }
    const T *begin() const { return inner; }
    const T *end() const { return inner + len; }
};

} // namespace soc