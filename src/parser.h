#ifndef PARSER_H
#define PARSER_H

#include "mpc.h"
#include <stdbool.h>

typedef struct {
    mpc_parser_t *trace_file;
    mpc_parser_t *trace;
    mpc_parser_t *core_num;
    mpc_parser_t *address;
    mpc_parser_t *instruction;
    mpc_parser_t *hex;
    mpc_parser_t *disassembly;
} parser_t;

parser_t parser_init();
void parser_deinit(parser_t *);

#endif
