#include <soclib/soclib.hpp>
#include <cstdio>

struct FileStats {
    soc::str_view name;
    size_t lines = 0;
    size_t words = 0;
    size_t bytes = 0;
};

static FileStats count_stats(soc::str_view name, soc::str_view content) {
    FileStats stats{ name, 0, 0, content.len };
    bool in_word = false;

    for (size_t i = 0; i < content.len; ++i) {
        char ch = *(content.inner + i);
        if (ch == '\n') {
            ++stats.lines;
        }
        bool is_space = (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
        if (is_space) {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            ++stats.words;
        }
    }
    return stats;
}

int main(int argc, char **argv) {
    soc::Timer timer;
    soc::Args args(argc, argv);

    bool verbose = false;
    soc::str_view out_path;
    soc::Vec<soc::str_view> inputs;

    // 1. Parse flags, options, and positional file paths
    while (args.has_next()) {
        if (args.flag("--verbose", "-v")) {
            verbose = true;
        } else if (args.option("--out", "-o", out_path)) {
        } else {
            inputs.push(args.next());
        }
    }

    if (inputs.len == 0) {
        soc::str usage;
        usage.push("Usage: ", soc::path::filename(args.program_name()),
                   " [-v|--verbose] [-o|--out <report.txt>] <file1> [file2 ...]\n");
        std::fwrite(usage.cstr(), 1, usage.len, stderr);
        return 1;
    }

    // 2. Use a StackArena for zero-heap-allocation stats records
    soc::StackArena<4096> scratch;
    FileStats *records = scratch.alloc<FileStats>(inputs.len);
    if (!records) {
        std::fprintf(stderr, "Error: too many input files for stack scratch buffer\n");
        return 1;
    }

    soc::str file_buf;
    FileStats total{ "TOTAL", 0, 0, 0 };

    // 3. Read & analyze each file
    for (size_t i = 0; i < inputs.len; ++i) {
        soc::str_view path_sv = inputs.at(i);

        // Ensure null-terminated path for OS/File APIs
        soc::str path_cstr;
        path_cstr.append(path_sv);

        if (!soc::os::is_file(path_cstr.cstr())) {
            soc::str err;
            err.push("Skipping missing/non-regular file: ", path_sv, "\n");
            std::fwrite(err.cstr(), 1, err.len, stderr);
            continue;
        }

        file_buf.clear();
        if (!soc::File::read_all(path_cstr.cstr(), file_buf)) {
            std::fprintf(stderr, "Failed to read: %s\n", path_cstr.cstr());
            continue;
        }

        FileStats st = count_stats(soc::path::filename(path_sv), file_buf.view());
        *(records + i) = st;

        total.lines += st.lines;
        total.words += st.words;
        total.bytes += st.bytes;
    }

    // 4. Format the summary report into a single soc::str
    soc::str report;
    for (size_t i = 0; i < inputs.len; ++i) {
        FileStats *st = records + i;
        if (st->name.len == 0) continue;
        report.push(st->lines, "\t", st->words, "\t", st->bytes, "\t", st->name, "\n");
    }
    report.push("----------------------------------------\n");
    report.push(total.lines, "\t", total.words, "\t", total.bytes, "\t", total.name, "\n");

    if (verbose) {
        soc::str cwd_buf;
        soc::os::cwd(cwd_buf);
        report.push("\nWorking dir : ", cwd_buf.view(),
                    "\nElapsed     : ", timer.elapsed_ms(), " ms\n");
    }

    // 5. Print to stdout and optionally save report via soc::File
    std::fwrite(report.cstr(), 1, report.len, stdout);

    if (out_path.len > 0) {
        soc::str out_cstr;
        out_cstr.append(out_path);
        if (soc::File::write_all(out_cstr.cstr(), report.view())) {
            if (verbose) {
                std::printf("Report saved to: %s\n", out_cstr.cstr());
            }
        } else {
            std::fprintf(stderr, "Failed to write report to: %s\n", out_cstr.cstr());
            return 1;
        }
    }

    return 0;
}