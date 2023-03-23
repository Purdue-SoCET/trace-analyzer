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

    const char *file = argv[1];
    // Check that file exists
    if (access(file, R_OK)) {
        printf("Could not open file %s!\n", file);
        return EXIT_FAILURE;
    }
    parser_t p = parser_t();

    mpc_result_t r;
    // TODO: Extremely slow. Can probably multithread this
    // Read line by line then use p.trace
    if (mpc_parse_contents(file, p.trace_file, &r)) {
        mpc_ast_t *ast = (mpc_ast_t *)r.output;
        Analyzer analyzer(ast->children_num);
        for (int i = 0; i < ast->children_num; i++) {
            /*analyzer.add_instr_hex(
                ast->children[i]->children[INSTRUCTION_IDX]->contents);
            analyzer.add_instr_pc(
                ast->children[i]->children[ADDRESS_IDX]->contents);*/
            mpc_ast_print(ast->children[i]->children[INSTRUCTION_IDX]);
        }
        analyzer.analyze();
        mpc_ast_delete((mpc_ast_t *)r.output);
    } else {
        printf("Could not parse file %s: %s", file, mpc_err_string(r.error));
        mpc_err_delete(r.error);
    }

    return EXIT_SUCCESS;
}
