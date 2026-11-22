# soclib (Structs Over Classes)

> *A C++ library for those who want the benefits of OOP while staying as far away from OOP as humanly possible.*

`soclib` is a lightweight, zero-overhead C++17 base library built directly on top of the C standard library and POSIX/OS primitives. Instead of hiding memory behind `private:` walls, deep inheritance trees, exceptions, or RTTI, `soclib` exposes **plain, transparent structs with attached methods**—giving you C's predictable, line-by-line execution with modern C++ type safety and scoped cleanup.

## Design Philosophy

* **Structs Over Classes:** Data fields (`inner`, `len`, `cap`, `offset`) are public and inspectable. Methods exist purely as ergonomic, type-safe shorthands around raw memory operations.
* **Built on C, Not STL Bloat:** Wraps `<cstdio>`, `<cstdlib>`, `<cstring>`, and OS syscalls directly (`-fno-exceptions -fno-rtti`) for fast compile times and deterministic behavior.
* **Explicit Memory Lifetimes:** Supports stack-backed arenas (`StackArena<N>`), heap arenas (`Arena`), and move-only RAII containers (`str`, `Vec<T>`, `File`) with zero hidden copies.
* **Decoupled Formatting & I/O:** String formatting (`<<` chaining and Lua-style variadic `.push(...)`) happens purely in memory via `soc::str`, completely separate from file or terminal I/O.

## Modules

| Header | Purpose |
| :--- | :--- |
| `<soclib/str.hpp>` | Owning dynamic `soc::str` (C-compatible `\0` terminated) and non-owning zero-copy `soc::str_view` slices. |
| `<soclib/vec.hpp>` | Generic stretchy buffer `soc::Vec<T>` and non-owning `soc::Slice<T>` views. |
| `<soclib/arena.hpp>` | Linear bump allocator (`soc::Arena`) and zero-heap `soc::StackArena<Bytes>` with `save()`/`restore()` checkpoints. |
| `<soclib/file.hpp>` | RAII wrapper around `std::FILE*` (`soc::File`) with one-shot `read_all` and `write_all` helpers. |
| `<soclib/cli.hpp>` | Zero-allocation `argc`/`argv` cursor (`soc::Args`) for flags, options, and positional arguments. |
| `<soclib/os.hpp>` | Cross-platform path slicing (`soc::path`), environment/filesystem queries (`soc::os`), and `soc::Timer`. |
| `<soclib/soclib.hpp>` | Umbrella header including all core `soclib` modules. |

## Quick Example

```cpp
#include <soclib/soclib.hpp>
#include <cstdio>

int main(int argc, char **argv) {
    soc::Timer timer;
    soc::Args args(argc, argv);

    // Zero-heap stack scratch arena
    soc::StackArena<1024> scratch;
    int *numbers = scratch.alloc<int>(4);
    *(numbers + 0) = 42;

    // Type-safe, decoupled string formatting
    soc::str msg;
    msg.push("Program: ", soc::path::filename(args.program_name()),
             " | Value: ", *(numbers + 0),
             " | Elapsed: ", timer.elapsed_ms(), " ms\n");

    // Direct C-compatible output
    std::fwrite(msg.cstr(), 1, msg.len, stdout);
    return 0;
}
```

## Building & Testing

`soclib` uses CMake (3.20+) and CTest. Unit tests compile with AddressSanitizer and UndefinedBehaviorSanitizer (`-fsanitize=address,undefined`) enabled by default.

```bash
# Configure and build with Ninja
cmake -B build -G Ninja
cmake --build build

# Run the modular CTest suite (or run ./test.sh)
ctest --test-dir build --output-on-failure
```

### Integrating via CMake

Add `soclib` as a subdirectory in your own `CMakeLists.txt` and link against the `soclib` target:

```cmake
add_subdirectory(soclib)
target_link_libraries(your_app PRIVATE soclib)
```

## License

Licensed under the **BSD 3-Clause License** — see [LICENSE](LICENSE) for details.
