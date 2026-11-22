#pragma once

#include <soclib/str.hpp>
#include <cstdint>

namespace soc {

namespace path {
    // Zero-allocation path slicing (handles both '/' and '\')
    str_view filename(str_view p);
    str_view extension(str_view p);
    str_view parent(str_view p);
} // namespace path

namespace os {
    // Environment & working directory (appends into 'out')
    bool env(const char *name, str &out);
    bool cwd(str &out);

    // Filesystem queries & directory creation
    bool exists(const char *p);
    bool is_dir(const char *p);
    bool is_file(const char *p);
    bool make_dir(const char *p);

    // Monotonic high-resolution clock (nanoseconds)
    uint64_t now_ns();
} // namespace os

struct Timer {
    uint64_t start_ns = 0;

    Timer() : start_ns(os::now_ns()) {}

    void reset() { start_ns = os::now_ns(); }
    uint64_t elapsed_ns() const { return os::now_ns() - start_ns; }
    double elapsed_ms() const { return static_cast<double>(elapsed_ns()) / 1'000'000.0; }
};

} // namespace soc