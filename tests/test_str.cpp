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
    // 1. Slicing & view equality
    soc::str_view full = "hello world";
    soc::str_view sub = full.slice(6, 5);
    CHECK(sub.equals("world"));
    CHECK(full.starts_with("hello"));

    // 2. Dynamic growth & variadic push formatting
    soc::str s;
    CHECK(s.append("HP: "));
    s.push(100, "/", 250, " (", true, ")");
    CHECK(s.view().equals("HP: 100/250 (true)"));
    CHECK(std::strcmp(s.cstr(), "HP: 100/250 (true)") == 0);

    // 3. Stateless fmt wrappers (hex, bin, upper, lower, pad)
    soc::str f;
    f.push(soc::fmt::hex(255), " | ",
           soc::fmt::hex(255, false, true), " | ",
           soc::fmt::bin(10), " | ",
           soc::fmt::upper("soclib"), " | ",
           soc::fmt::lower("ABC"), " | ",
           soc::fmt::pad_left("42", 4, '0'));
    CHECK(f.view().equals("0xff | FF | 0b1010 | SOCLIB | abc | 0042"));

    // 4. Move semantics & clear
    soc::str moved = static_cast<soc::str &&>(s);
    CHECK(s.inner == nullptr && s.len == 0);
    CHECK(moved.view().equals("HP: 100/250 (true)"));

    moved.clear();
    CHECK(moved.len == 0 && *moved.cstr() == '\0');

    std::printf("All str tests passed!\n");
    return 0;
}