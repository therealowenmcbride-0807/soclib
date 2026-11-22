#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>
#include <type_traits>

namespace soc {

struct Arena {
    uint8_t *base = nullptr;
    size_t cap = 0;
    size_t offset = 0;
    bool owns_memory = false;

    Arena() = default;
    Arena(uint8_t *buf, size_t capacity)
        : base(buf), cap(capacity), offset(0), owns_memory(false) {}

    Arena(const Arena &) = delete;
    Arena &operator=(const Arena &) = delete;

    Arena(Arena &&other) noexcept
        : base(other.base), cap(other.cap), offset(other.offset), owns_memory(other.owns_memory) {
        other.base = nullptr;
        other.cap = 0;
        other.offset = 0;
        other.owns_memory = false;
    }

    Arena &operator=(Arena &&other) noexcept {
        if (this != &other) {
            if (owns_memory) std::free(base);
            base = other.base;
            cap = other.cap;
            offset = other.offset;
            owns_memory = other.owns_memory;
            other.base = nullptr;
            other.cap = 0;
            other.offset = 0;
            other.owns_memory = false;
        }
        return *this;
    }

    ~Arena() {
        if (owns_memory) std::free(base);
    }

    // Create a heap-backed Arena
    static Arena create_heap(size_t capacity) {
        Arena a;
        a.base = static_cast<uint8_t *>(std::malloc(capacity));
        if (a.base) {
            a.cap = capacity;
            a.owns_memory = true;
        }
        return a;
    }

    // Core bump-allocation primitive
    void *alloc_raw(size_t bytes, size_t align = alignof(std::max_align_t)) {
        if (!base || bytes == 0) return nullptr;

        // Power-of-two alignment bump
        size_t mask = align - 1;
        size_t aligned = (offset + mask) & ~mask;

        if (aligned + bytes > cap) return nullptr; // Out of arena space

        void *ptr = base + aligned;
        offset = aligned + bytes;
        return ptr;
    }

    // Typed allocation helper (zero-initialized for POD types)
    template <typename T>
    T *alloc(size_t count = 1) {
        static_assert(std::is_trivially_destructible_v<T>,
                      "Arena only manages trivially destructible types (no per-item destructors)");
        void *mem = alloc_raw(sizeof(T) * count, alignof(T));
        if (!mem) return nullptr;

        T *typed = static_cast<T *>(mem);
        for (size_t i = 0; i < count; ++i) {
            new (typed + i) T{};
        }
        return typed;
    }

    // Save/restore checkpoints for temporary scratch scopes!
    size_t save() const { return offset; }
    void restore(size_t checkpoint) {
        assert(checkpoint <= offset);
        offset = checkpoint;
    }

    void reset() { offset = 0; }
    size_t remaining() const { return cap - offset; }
};

// Stack-allocated Arena: zero malloc/free calls!
template <size_t Bytes>
struct StackArena : Arena {
    alignas(std::max_align_t) std::array<uint8_t, Bytes> storage;

    StackArena() : Arena(storage.data(), Bytes) {}
};

} // namespace soc