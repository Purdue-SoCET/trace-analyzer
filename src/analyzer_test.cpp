#include "analyzer.hpp"
#include "parser.hpp"
#include "gtest/gtest.h"

TEST(AnalyzerTest, TestAnalyze) {
    // clang-format off
    std::string trace =
        "core          0: 0x0000000000008400 (0x00010117) auipc sp,      16\n"
        "core          0: 0x0000000000008404 (0x00010113) addi sp, sp,     0\n"
        "core          0: 0x0000000000008408 (0x38a040ef) jal ra, pc +    17290\n"
        "core          0: 0x000000000000c792 (0xfe010113) addi sp, sp,   -32\n"
        "core          0: 0x000000000000c794 (0x00112e23) sw ra,    28(sp)\n"
        "core          0: 0x000000000000c796 (0x00000097) auipc ra,       0\n"
        "core          0: 0x000000000000c79a (0xd04080e7) jalr ra, ra,  -764\n"
        "core          0: 0x000000000000c49a (0x00800793) addi a5, zero,     8\n"
        "core          0: 0x000000000000c49c (0x30079073) csrrw zero, mstatus, a5\n"
        "core          0: 0x000000000000c4a0 (0x00000797) auipc a5,       0";
    // clang-format on
    parser_t p = parser_t();
    mpc_result_t r;
    EXPECT_TRUE(mpc_parse("input", trace.c_str(), p.trace_file, &r));
    mpc_ast_t *ast = (mpc_ast_t *)r.output;

    std::vector<std::string> instrs;
    instrs.reserve(ast->children_num);
    std::vector<std::string> pcs;
    pcs.reserve(ast->children_num);
    for (int i = 0; i < ast->children_num; i++) {
        instrs.push_back(ast->children[i]->children[INSTRUCTION_IDX]->contents);
        pcs.push_back(ast->children[i]->children[ADDRESS_IDX]->contents);
    }

    Analyzer analyzer(instrs, pcs, CMAKE_TXT_DIR "/tests/nsichneu");
    EXPECT_TRUE(analyzer.analyze());
}

TEST(AnalyzerTest, TestInvalidBinary) {
    // clang-format off
    std::string trace =
        "core          0: 0x0000000000008400 (0x00010117) auipc sp,      16\n"
        "core          0: 0x0000000000008404 (0x00010113) addi sp, sp,     0\n"
        "core          0: 0x0000000000008408 (0x38a040ef) jal ra, pc +    17290\n"
        "core          0: 0x000000000000c792 (0xfe010113) addi sp, sp,   -32\n"
        "core          0: 0x000000000000c794 (0x00112e23) sw ra,    28(sp)\n"
        "core          0: 0x000000000000c796 (0x00000097) auipc ra,       0\n"
        "core          0: 0x000000000000c79a (0xd04080e7) jalr ra, ra,  -764\n"
        "core          0: 0x000000000000c49a (0x00800793) addi a5, zero,     8\n"
        "core          0: 0x000000000000c49c (0x30079073) csrrw zero, mstatus, a5\n"
        "core          0: 0x000000000000c4a0 (0x00000797) auipc a5,       0";
    // clang-format on
    parser_t p = parser_t();
    mpc_result_t r;
    EXPECT_TRUE(mpc_parse("input", trace.c_str(), p.trace_file, &r));
    mpc_ast_t *ast = (mpc_ast_t *)r.output;

    std::vector<std::string> instrs;
    instrs.reserve(ast->children_num);
    std::vector<std::string> pcs;
    pcs.reserve(ast->children_num);
    for (int i = 0; i < ast->children_num; i++) {
        instrs.push_back(ast->children[i]->children[INSTRUCTION_IDX]->contents);
        pcs.push_back(ast->children[i]->children[ADDRESS_IDX]->contents);
    }

    EXPECT_DEATH(Analyzer(instrs, pcs, "not a real file"), "Failed to create");
}
