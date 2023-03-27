#include "parser.hpp"
#include "src/analyzer.hpp"
#include <unistd.h>

int main(int argc, const char **argv) {
    if (argc < 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        printf("usage: %s TRACE_FILE [options]\n\n"
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

    mpc_result_t r;
    // TODO: Extremely slow. Can probably multithread this
    // Read line by line then use p.trace
    if (mpc_parse_contents(trace_file, p.trace_file, &r)) {
        mpc_ast_t *ast = (mpc_ast_t *)r.output;
        std::vector<std::string> instrs;
        std::vector<std::string> pcs;
        for (int i = 0; i < ast->children_num; i++) {
            instrs.push_back(
                ast->children[i]->children[INSTRUCTION_IDX]->contents);
            pcs.push_back(ast->children[i]->children[ADDRESS_IDX]->contents);
        }
        Analyzer analyzer(instrs, pcs, binary_file);
        analyzer.analyze();
        mpc_ast_delete((mpc_ast_t *)r.output);
    } else {
        printf("Could not parse file %s: %s", trace_file,
               mpc_err_string(r.error));
        mpc_err_delete(r.error);
    }

    return EXIT_SUCCESS;
}
