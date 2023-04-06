#ifndef PARSER_HPP
#define PARSER_HPP

extern "C" {
#include "mpc.h"
}
#include <stdbool.h>

#define ADDRESS_IDX 2
#define INSTRUCTION_IDX 4

/*
 * Parser object to maintain handles for each subparser.
 */
typedef struct parser_t {
    mpc_parser_t *trace_file;
    mpc_parser_t *trace;
    mpc_parser_t *core_num;
    mpc_parser_t *address;
    mpc_parser_t *instruction;
    mpc_parser_t *hex;
    mpc_parser_t *disassembly;

    parser_t();
    ~parser_t();
} parser_t;

#endif
