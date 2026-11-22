#include <soclib/arena.hpp>
#include <cstdio>
#include <cstdlib>

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::fprintf(stderr, "FAIL: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
        std::exit(1); \
    } \
} while (0)

struct Point {
    double x;
    double y;
};

static void fill_points(soc::Arena &arena, size_t count) {
    Point *pts = arena.alloc<Point>(count);
    CHECK(pts != nullptr);
    for (size_t i = 0; i < count; ++i) {
        (pts + i)->x = static_cast<double>(i);
        (pts + i)->y = static_cast<double>(i * 2);
    }
    CHECK((pts + 2)->x == 2.0 && (pts + 2)->y == 4.0);
}

int main() {
    // 1. Stack-backed Arena (0 heap allocations)
    soc::StackArena<256> stack_arena;
    CHECK(!stack_arena.owns_memory);
    CHECK(stack_arena.remaining() == 256);

    int *num = stack_arena.alloc<int>();
    CHECK(num != nullptr && *num == 0);
    *num = 99;

    // 2. Checkpoint save/restore
    size_t mark = stack_arena.save();
    fill_points(stack_arena, 4); // Takes 4 * 16 = 64 bytes
    CHECK(stack_arena.offset > mark);
    stack_arena.restore(mark);
    CHECK(stack_arena.offset == mark);
    CHECK(*num == 99); // Earlier allocation untouched!

    // 3. Bounds check when arena is full
    CHECK(stack_arena.alloc_raw(1024) == nullptr);

    // 4. Heap-backed Arena (freed automatically on scope exit)
    soc::Arena heap_arena = soc::Arena::create_heap(4096);
    CHECK(heap_arena.owns_memory);
    fill_points(heap_arena, 100);
    heap_arena.reset();
    CHECK(heap_arena.offset == 0 && heap_arena.remaining() == 4096);

    std::printf("All arena tests passed!\n");
    return 0;
}