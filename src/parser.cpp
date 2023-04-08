#include "parser.hpp"
// General format of instruction trace:
//    <core number>          <address> (<instruction>) <disassembly>
// core          0: 0x0000000000008400 (0x00010117) auipc sp,      16

// Builds the parser for instruction traces.
parser_t::parser_t()
    : trace_file(mpc_new("trace_file")), trace(mpc_new("trace")), core_num(mpc_new("core_num")),
      address(mpc_new("address")), instruction(mpc_new("instruction")), hex(mpc_new("hex")),
      disassembly(mpc_new("disassembly")) {

    /* clang-format off */
    mpc_err_t *err;
    if ((err = mpca_lang(MPCA_LANG_DEFAULT,
                    " trace_file : <trace>*;"
                    " trace : <core_num> ':' <address> '(' <instruction> ')' <disassembly>;"
                    " core_num : \"core\" /[0-9]+/;"
                    " address : <hex>;"
                    " instruction : <hex>;"
                    " hex : /0[xX][0-9a-fA-F]+/;"
                    " disassembly : /.*/ ;",
                    this->trace_file, this->trace, this->core_num, this->address, this->instruction, this->hex, this->disassembly, NULL))) {
        mpc_err_print(err);
        mpc_err_delete(err);
        exit(1) ;
    }
    /* clang-format on */
}

parser_t::~parser_t() {
    mpc_cleanup(7, this->trace_file, this->trace, this->core_num, this->address, this->instruction,
                this->hex, this->disassembly);
}
