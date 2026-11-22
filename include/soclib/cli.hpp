#pragma once

#include <soclib/str.hpp>

namespace soc {

struct Args {
    int argc = 0;
    char **argv = nullptr;
    int index = 1; // Skips argv[0] (the program name) by default

    Args() = default;
    Args(int ac, char **av) : argc(ac), argv(av), index(1) {}

    str_view program_name() const {
        return (argc > 0 && argv) ? str_view(*(argv + 0)) : str_view();
    }

    bool has_next() const {
        return argv != nullptr && index < argc;
    }

    str_view peek() const {
        return has_next() ? str_view(*(argv + index)) : str_view();
    }

    str_view next() {
        if (!has_next()) return {};
        str_view current(*(argv + index));
        ++index;
        return current;
    }

    // Matches a boolean flag (e.g., flag("--verbose", "-v")) and consumes it
    bool flag(str_view long_name, str_view short_name = {}) {
        if (!has_next()) return false;
        str_view cur = peek();
        if ((!long_name.inner && !short_name.inner)) return false;

        if ((long_name.len > 0 && cur.equals(long_name)) ||
            (short_name.len > 0 && cur.equals(short_name))) {
            ++index;
            return true;
        }
        return false;
    }

    // Matches "--key value", "-k value", OR "--key=value" and writes into 'out'
    bool option(str_view long_name, str_view short_name, str_view &out) {
        if (!has_next()) return false;
        str_view cur = peek();

        // 1. Check exact match: "--output" or "-o" followed by next arg
        if ((long_name.len > 0 && cur.equals(long_name)) ||
            (short_name.len > 0 && cur.equals(short_name))) {
            if (index + 1 >= argc) return false; // Missing value after flag
            ++index;
            out = next();
            return true;
        }

        // 2. Check "--output=value" inline syntax
        if (long_name.len > 0 && cur.starts_with(long_name) &&
            cur.len > long_name.len && *(cur.inner + long_name.len) == '=') {
            out = cur.slice(long_name.len + 1, cur.len - (long_name.len + 1));
            ++index;
            return true;
        }

        return false;
    }
};

} // namespace soc