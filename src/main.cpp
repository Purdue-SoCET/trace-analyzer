#include "analyzer.hpp"
#include "cli.hpp"
#include "parser.hpp"
#include <execution>
#include <fstream>
#include <iostream>
#include <mutex>
#include <unistd.h>

int main(int argc, const char **argv) {
    Cli cli(argc, argv);

    // Check that file exists
    if (access(cli.trace_file, R_OK)) {
        printf("Could not open file %s!\n", cli.trace_file);
        return EXIT_FAILURE;
    }
    parser_t p = parser_t();
    std::ifstream trace_if(cli.trace_file);
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
    Analyzer analyzer(instrs, pcs, cli.binary_file);
    analyzer.analyze();

    switch (cli.format) {
    case Cli::format_normal_e:
        analyzer.displayStatistics();
        break;
    case Cli::format_json_e:
        analyzer.displayStatisticsJson();
        break;
    case Cli::format_matlab_e:
        analyzer.displayStatisticsMatlab();
        break;
    }

    return EXIT_SUCCESS;
}
