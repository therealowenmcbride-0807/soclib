#include <soclib/vec.hpp>
#include <soclib/str.hpp>
#include <cstdio>
#include <cstdlib>

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::fprintf(stderr, "FAIL: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
        std::exit(1); \
    } \
} while (0)

int main() {
    // 1. Plain integers (uses fast std::realloc path)
    soc::Vec<int> nums;
    for (int i = 0; i < 20; ++i) {
        CHECK(nums.push(i * 10));
    }
    CHECK(nums.len == 20);
    CHECK(nums.at(5) == 50);
    CHECK(nums.back() == 190);

    nums.pop();
    CHECK(nums.len == 19);

    // 2. Non-owning Slice<int> view
    soc::Slice<int> sub = nums.slice().subslice(2, 3);
    CHECK(sub.len == 3 && sub.at(0) == 20 && sub.at(2) == 40);

    // 3. Nested non-trivial structs (Vec<soc::str>)
    soc::Vec<soc::str> words;
    soc::str a; a.append("hello");
    soc::str b; b.append("soclib");
    CHECK(words.push(static_cast<soc::str &&>(a)));
    CHECK(words.push(static_cast<soc::str &&>(b)));
    CHECK(words.len == 2);
    CHECK(words.at(0).view().equals("hello"));
    CHECK(words.at(1).view().equals("soclib"));

    std::printf("All vec tests passed!\n");
    return 0;
}