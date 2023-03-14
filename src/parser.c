#include "parser.h"
// General format of instruction trace:
//    <core number>          <address> (<instruction>) <disassembly>
// core          0: 0x0000000000008400 (0x00010117) auipc sp,      16

// Builds the parser for instruction traces.
parser_t parser_init() {
    parser_t p;
    p.trace_file = mpc_new("trace_file");
    p.trace = mpc_new("trace");
    p.core_num = mpc_new("core_num");
    p.address = mpc_new("address");
    p.instruction = mpc_new("instruction");
    p.hex = mpc_new("hex");
    p.disassembly = mpc_new("disassembly");

    /* clang-format off */
    mpc_err_t *err;
    if ((err = mpca_lang(MPCA_LANG_DEFAULT,
                    " trace_file : <trace>*;"
                    " trace : /^/<core_num> ':' <address> '(' <instruction> ')' <disassembly> /$/;"
                    " core_num : \"core\" /[0-9]+/;"
                    " address : <hex>;"
                    " instruction : <hex>;"
                    " hex : '0' ('x' | 'X') /[0-9a-fA-F]+/;"
                    " disassembly : /.*/ ;",
                    p.trace_file, p.trace, p.core_num, p.address, p.instruction, p.hex, p.disassembly, NULL))) {
        mpc_err_print(err);
        mpc_err_delete(err);
        exit(1) ;
    }
    /* clang-format on */

    mpc_result_t r;
    return p;
}

void parser_deinit(parser_t *p) {
    mpc_cleanup(6, p->trace_file, p->trace, p->core_num, p->address,
                p->instruction, p->hex, p->disassembly);
}
