#include <soclib/file.hpp>

namespace soc {

File::File(File &&other) noexcept : handle(other.handle) {
    other.handle = nullptr;
}

File &File::operator=(File &&other) noexcept {
    if (this != &other) {
        close();
        handle = other.handle;
        other.handle = nullptr;
    }
    return *this;
}

File::~File() {
    close();
}

File File::open(const char *path, const char *mode) {
    return File(std::fopen(path, mode));
}

void File::close() {
    if (handle) {
        std::fclose(handle);
        handle = nullptr;
    }
}

size_t File::write(str_view data) {
    if (!handle || data.len == 0) return 0;
    return std::fwrite(data.inner, 1, data.len, handle);
}

size_t File::read(void *dst, size_t bytes) {
    if (!handle || bytes == 0) return 0;
    return std::fread(dst, 1, bytes, handle);
}

bool File::flush() {
    return handle && std::fflush(handle) == 0;
}

bool File::read_all(const char *path, str &out) {
    File f = File::open(path, "rb");
    if (!f.is_open()) return false;

    if (std::fseek(f.handle, 0, SEEK_END) != 0) return false;
    long file_size = std::ftell(f.handle);
    if (file_size < 0) return false;
    if (std::fseek(f.handle, 0, SEEK_SET) != 0) return false;

    size_t size = static_cast<size_t>(file_size);
    if (!out.grow(out.len + size)) return false;

    size_t bytes_read = std::fread(out.inner + out.len, 1, size, f.handle);
    out.len += bytes_read;
    out.inner[out.len] = '\0';

    return bytes_read == size;
}

bool File::write_all(const char *path, str_view data) {
    File f = File::open(path, "wb");
    if (!f.is_open()) return false;
    return f.write(data) == data.len;
}

} // namespace soc