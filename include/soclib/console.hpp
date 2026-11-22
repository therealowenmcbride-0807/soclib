#pragma once

#include <soclib/str.hpp>
#include <cstdio>

namespace soc {
namespace console {

enum class Color : uint8_t {
    Default = 39,
    Black = 30,
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    White = 37,
    BrightBlack = 90,
    BrightRed = 91,
    BrightGreen = 92,
    BrightYellow = 93,
    BrightBlue = 94,
    BrightMagenta = 95,
    BrightCyan = 96,
    BrightWhite = 97
};

enum class BgColor : uint8_t {
    Default = 49,
    Black = 40,
    Red = 41,
    Green = 42,
    Yellow = 43,
    Blue = 44,
    Magenta = 45,
    Cyan = 46,
    White = 47,
    BrightBlack = 100,
    BrightRed = 101,
    BrightGreen = 102,
    BrightYellow = 103,
    BrightBlue = 104,
    BrightMagenta = 105,
    BrightCyan = 106,
    BrightWhite = 107
};

enum class Attr : uint8_t {
    None = 0,
    Bold = 1,
    Dim = 2,
    Underline = 4
};

// Enables ANSI virtual terminal processing on Windows (no-op on Linux/macOS)
bool enable_ansi();

// Returns true if stream is attached to an interactive terminal
bool is_tty(std::FILE *stream = stdout);

// Screen & cursor control
void clear(std::FILE *stream = stdout);
void reset_style(std::FILE *stream = stdout);

// Stateless inline style wrapper for soc::str formatting
struct Styled {
    str_view text;
    Color fg = Color::Default;
    BgColor bg = BgColor::Default;
    Attr attr = Attr::None;
};

constexpr Styled style(str_view text, Color fg = Color::Default,
                       BgColor bg = BgColor::Default, Attr attr = Attr::None) {
    return { text, fg, bg, attr };
}

// Reads a full line from stream (default: stdin) into 'out', stripping trailing \r/\n.
// Returns false on EOF before any characters are read.
bool read_line(str &out, std::FILE *stream = stdin);

// Type-safe variadic output built on soc::str
template <typename... Args>
void print(Args &&...args) {
    str buf;
    buf.push(static_cast<Args &&>(args)...);
    if (buf.len > 0) {
        std::fwrite(buf.cstr(), 1, buf.len, stdout);
    }
}

template <typename... Args>
void println(Args &&...args) {
    str buf;
    buf.push(static_cast<Args &&>(args)...);
    buf.append(str_view("\n", 1));
    std::fwrite(buf.cstr(), 1, buf.len, stdout);
}

template <typename... Args>
void eprintln(Args &&...args) {
    str buf;
    buf.push(static_cast<Args &&>(args)...);
    buf.append(str_view("\n", 1));
    std::fwrite(buf.cstr(), 1, buf.len, stderr);
}

} // namespace console

// Formats Styled segments into any soc::str and automatically resets attributes afterward
str &operator<<(str &out, console::Styled s);

} // namespace soc