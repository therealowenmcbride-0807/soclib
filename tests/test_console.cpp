#include <soclib/console.hpp>
#include <cstdio>
#include <cstdlib>

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::fprintf(stderr, "FAIL: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
        std::exit(1); \
    } \
} while (0)

int main() {
    using namespace soc;

    // 1. Foreground color only
    str s1;
    s1 << console::style("OK", console::Color::Green);
    CHECK(s1.view().equals("\033[32mOK\033[0m"));

    // 2. Bold + Foreground + Background combined into one ANSI sequence
    str s2;
    s2 << console::style("ALERT", console::Color::BrightWhite,
                         console::BgColor::Red, console::Attr::Bold);
    CHECK(s2.view().equals("\033[1;97;41mALERT\033[0m"));

    // 3. Default style emits clean text with no escape overhead
    str s3;
    s3 << console::style("plain");
    CHECK(s3.view().equals("plain"));

    // 4. Test console::read_line() using an in-memory temporary FILE*
    std::FILE *tmp = std::tmpfile();
    CHECK(tmp != nullptr);

    str_view mock_input = "first line\r\nsecond line\n";
    std::fwrite(mock_input.inner, 1, mock_input.len, tmp);
    std::rewind(tmp);

    str line1;
    CHECK(console::read_line(line1, tmp));
    CHECK(line1.view().equals("first line"));

    str line2;
    CHECK(console::read_line(line2, tmp));
    CHECK(line2.view().equals("second line"));

    str eof_line;
    CHECK(!console::read_line(eof_line, tmp));

    std::fclose(tmp);

    std::printf("All console tests passed!\n");
    return 0;
}