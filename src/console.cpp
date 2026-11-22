#include <soclib/console.hpp>
#include <array>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <io.h>
#else
    #include <unistd.h>
#endif

namespace soc {
namespace console {

bool enable_ansi() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return false;
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return false;
    return SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
    return true;
#endif
}

bool is_tty(std::FILE *stream) {
    if (!stream) return false;
#ifdef _WIN32
    return _isatty(_fileno(stream)) != 0;
#else
    return isatty(fileno(stream)) != 0;
#endif
}

void clear(std::FILE *stream) {
    if (!stream) return;
    str_view seq("\033[2J\033[H", 7);
    std::fwrite(seq.inner, 1, seq.len, stream);
    std::fflush(stream);
}

void reset_style(std::FILE *stream) {
    if (!stream) return;
    str_view seq("\033[0m", 4);
    std::fwrite(seq.inner, 1, seq.len, stream);
}

bool read_line(str &out, std::FILE *stream) {
    if (!stream) return false;

    std::array<char, 256> chunk;
    bool read_any = false;

    while (std::fgets(chunk.data(), static_cast<int>(chunk.size()), stream) != nullptr) {
        read_any = true;
        str_view part(chunk.data());
        out.append(part);

        // Stop once we hit a newline character
        if (part.len > 0 && *(part.inner + part.len - 1) == '\n') {
            break;
        }
    }

    if (!read_any) return false;

    // Strip trailing '\n' and '\r' cleanly
    while (out.len > 0) {
        char last = *(out.inner + out.len - 1);
        if (last == '\n' || last == '\r') {
            --out.len;
            *(out.inner + out.len) = '\0';
        } else {
            break;
        }
    }

    return true;
}

} // namespace console

str &operator<<(str &out, console::Styled s) {
    out.append(str_view("\033[", 2));
    bool has_code = false;

    if (s.attr != console::Attr::None) {
        out << static_cast<uint32_t>(s.attr);
        has_code = true;
    }
    if (s.fg != console::Color::Default) {
        if (has_code) out.append(str_view(";", 1));
        out << static_cast<uint32_t>(s.fg);
        has_code = true;
    }
    if (s.bg != console::BgColor::Default) {
        if (has_code) out.append(str_view(";", 1));
        out << static_cast<uint32_t>(s.bg);
        has_code = true;
    }

    // If no style flags were passed, emit plain text without empty escape sequences
    if (!has_code) {
        out.len -= 2;
        if (out.inner) *(out.inner + out.len) = '\0';
        out.append(s.text);
        return out;
    }

    out.push("m", s.text, "\033[0m");
    return out;
}

} // namespace soc