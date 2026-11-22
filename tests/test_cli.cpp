#include <soclib/cli.hpp>
#include <cstdio>
#include <cstdlib>

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::fprintf(stderr, "FAIL: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
        std::exit(1); \
    } \
} while (0)

int main() {
    char arg0[] = "mytool";
    char arg1[] = "-v";
    char arg2[] = "--output";
    char arg3[] = "out.bin";
    char arg4[] = "--mode=release";
    char arg5[] = "input.cpp";

    char *fake_argv[] = { arg0, arg1, arg2, arg3, arg4, arg5 };
    soc::Args args(6, fake_argv);

    CHECK(args.program_name().equals("mytool"));

    bool verbose = false;
    soc::str_view output;
    soc::str_view mode;
    soc::str_view positional;

    while (args.has_next()) {
        if (args.flag("--verbose", "-v")) {
            verbose = true;
        } else if (args.option("--output", "-o", output)) {
        } else if (args.option("--mode", "-m", mode)) {
        } else {
            positional = args.next();
        }
    }

    CHECK(verbose == true);
    CHECK(output.equals("out.bin"));
    CHECK(mode.equals("release"));
    CHECK(positional.equals("input.cpp"));
    CHECK(!args.has_next());

    std::printf("All cli tests passed!\n");
    return 0;
}