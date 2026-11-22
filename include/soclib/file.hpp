#pragma once

#include <soclib/str.hpp>
#include <cstdio>

namespace soc {

struct File {
    std::FILE *handle = nullptr;

    File() = default;
    explicit File(std::FILE *fp) : handle(fp) {}

    // Prevent accidental double-close of the same FILE*
    File(const File &) = delete;
    File &operator=(const File &) = delete;

    // Safe ownership transfer
    File(File &&other) noexcept;
    File &operator=(File &&other) noexcept;
    ~File();

    // Open & close (implemented in file.cpp)
    static File open(const char *path, const char *mode);
    void close();

    // Streaming read/write helpers
    size_t write(str_view data);
    size_t read(void *dst, size_t bytes);
    bool flush();

    // One-shot convenience helpers
    static bool read_all(const char *path, str &out);
    static bool write_all(const char *path, str_view data);

    bool is_open() const { return handle != nullptr; }
};

} // namespace soc