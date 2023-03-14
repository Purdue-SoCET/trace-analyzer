#include "parser.h"

int main(void) {
    parser_t p = parser_init();

    parser_deinit(&p);
    return EXIT_SUCCESS;
}
