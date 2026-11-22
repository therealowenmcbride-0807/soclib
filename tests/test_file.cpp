#include <soclib/file.hpp>
#include <cstdio>
#include <cstdlib>

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::fprintf(stderr, "FAIL: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
        std::exit(1); \
    } \
} while (0)

int main() {
    const char *tmp_path = "soclib_test_tmp.txt";

    // 1. One-shot write using soc::str formatting + str_view
    soc::str payload;
    payload.push("Line 1: ", 42, "\nLine 2: ", true, "\n");
    CHECK(soc::File::write_all(tmp_path, payload.view()));

    // 2. One-shot read directly into a fresh soc::str
    soc::str loaded;
    CHECK(soc::File::read_all(tmp_path, loaded));
    CHECK(loaded.view().equals(payload.view()));

    // 3. Streaming append & RAII move semantics
    {
        soc::File f = soc::File::open(tmp_path, "ab");
        CHECK(f.is_open());

        soc::File moved = static_cast<soc::File &&>(f);
        CHECK(!f.is_open());
        CHECK(moved.is_open());

        soc::str_view extra = "Line 3: appended\n";
        CHECK(moved.write(extra) == extra.len);
        CHECK(moved.flush());
        // 'moved' closes automatically at end of scope
    }

    // 4. Verify appended content + clean up temp file
    soc::str final_content;
    CHECK(soc::File::read_all(tmp_path, final_content));
    CHECK(final_content.view().equals("Line 1: 42\nLine 2: true\nLine 3: appended\n"));

    std::remove(tmp_path);

    std::printf("All file tests passed!\n");
    return 0;
}