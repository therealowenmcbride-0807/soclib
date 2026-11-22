#include <soclib/os.hpp>
#include <cstdio>
#include <cstdlib>

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::fprintf(stderr, "FAIL: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
        std::exit(1); \
    } \
} while (0)

int main() {
    // 1. Zero-allocation path slicing
    soc::str_view p = "/home/user/projects/soclib/src/os.cpp";
    CHECK(soc::path::filename(p).equals("os.cpp"));
    CHECK(soc::path::extension(p).equals(".cpp"));
    CHECK(soc::path::parent(p).equals("/home/user/projects/soclib/src"));
    CHECK(soc::path::extension(".gitignore").len == 0);

    // 2. Working directory & environment lookup
    soc::str dir;
    CHECK(soc::os::cwd(dir));
    CHECK(dir.len > 0);
    CHECK(soc::os::is_dir(dir.cstr()));
    CHECK(!soc::os::is_file(dir.cstr()));

    soc::str path_env;
    CHECK(soc::os::env("PATH", path_env));
    CHECK(path_env.len > 0);

    // 3. Monotonic timer sanity check
    soc::Timer timer;
    volatile int sum = 0;
    for (int i = 0; i < 10000; ++i) sum += i;
    CHECK(timer.elapsed_ns() > 0);

    std::printf("All os tests passed!\n");
    return 0;
}