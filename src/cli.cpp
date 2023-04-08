#include "cli.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

Cli::Cli(int argc, const char **argv)
    : trace_file(argv[1]), binary_file(argv[2]), format(format_normal_e) {
    bool help = argc < 3;
    for (int i = 3; i < argc; i++) {
        if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0)) {
            help = true;
        } else if (strncmp(argv[i], "--format=", 9) == 0) {
            if (strcmp(argv[i] + 9, "normal") == 0) {
                format = format_normal_e;
            } else if (strcmp(argv[i] + 9, "json") == 0) {
                format = format_json_e;
            } else if (strcmp(argv[i] + 9, "matlab") == 0) {
                format = format_matlab_e;
            } else if (strcmp(argv[i] + 9, "extension") == 0) {
                format = format_ext_e;
            } else {
                printf("Unknown format \"%s\", please use one of \"normal\", "
                       "\"json\", or \"matlab\".\n",
                       argv[i] + 9);
                exit(EXIT_FAILURE);
            }
        }
    }
    if (help) {
        // clang-format off
        printf("usage: %s TRACE_FILE BINARY_FILE [options]\n\n"
               "Options:\n"
               // TODO: will this assume function only called once?
               // Won't be deterministic (i.e. args different each time and we
               // can't tell from only instruction traces)
               "\t--format=<format>  Display statistics in <format> where format is one of \"normal\", \"json\", \"matlab\", or \"extension\". Defaults to \"normal\".\n"
               "\t--function=<f>  Only analyze function <f> (NOT IMPLEMENTED)\n",
               argv[0]);
        // clang-format on
        exit(EXIT_FAILURE);
    }
}
