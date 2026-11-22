#include <soclib/os.hpp>

#include <cstdlib>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <direct.h>
    #include <sys/stat.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <time.h>
    #include <limits.h>
#endif

namespace soc {

namespace path {

static bool is_sep(char ch) {
    return ch == '/' || ch == '\\';
}

str_view filename(str_view p) {
    if (p.len == 0) return {};
    size_t i = p.len;
    while (i > 0 && !is_sep(*(p.inner + i - 1))) {
        --i;
    }
    return p.slice(i, p.len - i);
}

str_view extension(str_view p) {
    str_view name = filename(p);
    if (name.len == 0) return {};
    for (size_t i = name.len; i > 0; --i) {
        char ch = *(name.inner + i - 1);
        if (ch == '.') {
            // Ignore leading dotfiles like ".gitignore"
            if (i - 1 == 0) return {};
            return name.slice(i - 1, name.len - (i - 1));
        }
    }
    return {};
}

str_view parent(str_view p) {
    if (p.len == 0) return {};
    size_t i = p.len;
    while (i > 0 && !is_sep(*(p.inner + i - 1))) {
        --i;
    }
    if (i == 0) return {};
    // Strip trailing separator unless it's the root "/"
    if (i > 1) --i;
    return p.slice(0, i);
}

} // namespace path

namespace os {

bool env(const char *name, str &out) {
    if (!name) return false;
#ifdef _WIN32
    char *buf = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, name) != 0 || !buf) return false;
    bool ok = out.append(str_view(buf));
    std::free(buf);
    return ok;
#else
    const char *val = std::getenv(name);
    if (!val) return false;
    return out.append(str_view(val));
#endif
}

bool cwd(str &out) {
#ifdef _WIN32
    char buf[MAX_PATH];
    if (!_getcwd(buf, MAX_PATH)) return false;
    return out.append(str_view(buf));
#else
    char buf[PATH_MAX];
    if (!getcwd(buf, sizeof(buf))) return false;
    return out.append(str_view(buf));
#endif
}

bool exists(const char *p) {
    if (!p) return false;
#ifdef _WIN32
    struct _stat info;
    return _stat(p, &info) == 0;
#else
    struct stat info;
    return stat(p, &info) == 0;
#endif
}

bool is_dir(const char *p) {
    if (!p) return false;
#ifdef _WIN32
    struct _stat info;
    return _stat(p, &info) == 0 && (info.st_mode & _S_IFDIR) != 0;
#else
    struct stat info;
    return stat(p, &info) == 0 && S_ISDIR(info.st_mode);
#endif
}

bool is_file(const char *p) {
    if (!p) return false;
#ifdef _WIN32
    struct _stat info;
    return _stat(p, &info) == 0 && (info.st_mode & _S_IFREG) != 0;
#else
    struct stat info;
    return stat(p, &info) == 0 && S_ISREG(info.st_mode);
#endif
}

bool make_dir(const char *p) {
    if (!p) return false;
#ifdef _WIN32
    return _mkdir(p) == 0;
#else
    return mkdir(p, 0755) == 0;
#endif
}

uint64_t now_ns() {
#ifdef _WIN32
    static LARGE_INTEGER freq = [] {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        return f;
    }();
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<uint64_t>((counter.QuadPart * 1'000'000'000ULL) / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL +
           static_cast<uint64_t>(ts.tv_nsec);
#endif
}

} // namespace os
} // namespace soc