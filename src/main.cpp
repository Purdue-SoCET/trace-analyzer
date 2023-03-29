#include "analyzer.hpp"
#include "parser.hpp"
#include <execution>
#include <fstream>
#include <iostream>
#include <mutex>
#include <unistd.h>

int main(int argc, const char **argv) {
    if (argc < 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        printf("usage: %s TRACE_FILE BINARY_FILE [options]\n\n"
               "Options:\n"
               // TODO: will this assume function only called once?
               // Won't be deterministic (i.e. args different each time and we
               // can't tell from only instruction traces)
               "\t--function=<f>  Only analyze function <f> (NOT IMPLEMENTED)"
               "\n",
               argv[0]);
        return EXIT_FAILURE;
    }

    const char *trace_file = argv[1];
    const char *binary_file = argv[2];
    // Check that file exists
    if (access(trace_file, R_OK)) {
        printf("Could not open file %s!\n", trace_file);
        return EXIT_FAILURE;
    }
    parser_t p = parser_t();
    std::ifstream trace_if(trace_file);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(trace_if, line)) {
        lines.push_back(line);
    }

    std::vector<std::string> instrs;
    std::vector<std::string> pcs;
    std::mutex m;
    std::for_each(
        std::execution::par, lines.begin(), lines.end(),
        [&](std::string_view s) {
            mpc_result_t r;
            if (mpc_parse("trace", s.data(), p.trace, &r)) {
                mpc_ast_t *ast = (mpc_ast_t *)r.output;
                {
                    std::lock_guard lock(m);
                    instrs.push_back(ast->children[INSTRUCTION_IDX]->contents);
                    pcs.push_back(ast->children[ADDRESS_IDX]->contents);
                }
                mpc_ast_delete((mpc_ast_t *)r.output);
            } else {
                printf("Could not parse trace %s: %s", s.data(),
                       mpc_err_string(r.error));
                mpc_err_delete(r.error);
            }
        });
    Analyzer analyzer(instrs, pcs, binary_file);
    analyzer.analyze();

    analyzer.displayStatistics();

    return EXIT_SUCCESS;
}
