extern "C" {
#include "parser.h"
}
#include <gtest/gtest.h>
#include <stdio.h>

TEST(ParserTest, TestTraceFile) {
    parser_t p = parser_init();
    mpc_result_t r;
    int parsed = mpc_parse(
        "trace",
        // clang-format off
        "core          0: 0x0000000000008400 (0x00010117) auipc sp,      16\n"
        "core          0: 0x0000000000008404 (0x00010113) addi sp, sp,     0\n"
        "core          0: 0x0000000000008408 (0x38a040ef) jal ra, pc +    17290\n"
        "core          0: 0x000000000000c792 (0xfe010113) addi sp, sp,   -32\n"
        "core          0: 0x000000000000c794 (0x00112e23) sw ra,    28(sp)\n"
        "core          0: 0x000000000000c796 (0x00000097) auipc ra,       0\n"
        "core          0: 0x000000000000c79a (0xd04080e7) jalr ra, ra,  -764\n"
        "core          0: 0x000000000000c49a (0x00800793) addi a5, zero,     8\n"
        "core          0: 0x000000000000c49c (0x30079073) csrrw zero, mstatus, a5\n"
        "core          0: 0x000000000000c4a0 (0x00000797) auipc a5,       0\n",
        // clang-format on
        p.trace_file, &r);
    EXPECT_TRUE(parsed);
    parser_deinit(&p);
}

TEST(ParserTest, TestTrace) {
    parser_t p = parser_init();
    mpc_result_t r;
    int parsed = mpc_parse(
        "trace",
        "core          0: 0x0000000000008400 (0x00010117) auipc sp,      16",
        p.trace, &r);
    EXPECT_TRUE(parsed);
    // TODO: flatten ast by removing unnecessary captures
    EXPECT_STREQ(((mpc_ast_t *)r.output)->children[1]->children[1]->contents,
                 "0");
    EXPECT_STREQ(((mpc_ast_t *)r.output)->children[3]->contents,
                 "0x0000000000008400");
    EXPECT_STREQ(((mpc_ast_t *)r.output)->children[5]->contents, "0x00010117");
    EXPECT_STREQ(((mpc_ast_t *)r.output)->children[7]->contents,
                 "auipc sp,      16");
    parser_deinit(&p);
}

TEST(ParserTest, TestCoreNum) {
    parser_t p = parser_init();
    mpc_result_t r;
    int parsed = mpc_parse("trace", "core          0", p.core_num, &r);
    EXPECT_TRUE(parsed);
    EXPECT_STREQ(((mpc_ast_t *)r.output)->children[1]->contents, "0");
    parser_deinit(&p);
}

TEST(ParserTest, TestAddress) {
    parser_t p = parser_init();
    mpc_result_t r;
    int parsed = mpc_parse("trace", "0x0000000000008400", p.address, &r);
    EXPECT_TRUE(parsed);
    EXPECT_STREQ(((mpc_ast_t *)r.output)->contents, "0x0000000000008400");
    parser_deinit(&p);
}

TEST(ParserTest, TestInstruction) {
    parser_t p = parser_init();
    mpc_result_t r;
    int parsed = mpc_parse("trace", "0x00010117", p.instruction, &r);
    EXPECT_TRUE(parsed);
    EXPECT_STREQ(((mpc_ast_t *)r.output)->contents, "0x00010117");
    parser_deinit(&p);
}

TEST(ParserTest, TestHex) {
    parser_t p = parser_init();
    mpc_result_t r;
    int parsed = mpc_parse("trace", "0x4343241afbc", p.hex, &r);
    EXPECT_TRUE(parsed);
    EXPECT_STREQ(((mpc_ast_t *)r.output)->contents, "0x4343241afbc");
    parsed = mpc_parse("trace", "0X4234", p.hex, &r);
    EXPECT_TRUE(parsed);
    EXPECT_STREQ(((mpc_ast_t *)r.output)->contents, "0X4234");
    parsed = mpc_parse("trace", "    ", p.hex, &r);
    EXPECT_FALSE(parsed);
    parsed = mpc_parse("trace", "0xy", p.hex, &r);
    EXPECT_FALSE(parsed);
    parsed = mpc_parse("trace", "0y", p.hex, &r);
    EXPECT_FALSE(parsed);
    parsed = mpc_parse("trace", "0", p.hex, &r);
    EXPECT_FALSE(parsed);
    parsed = mpc_parse("trace", "0X", p.hex, &r);
    EXPECT_FALSE(parsed);
    parsed = mpc_parse("trace", "0xg", p.hex, &r);
    EXPECT_FALSE(parsed);
    parsed = mpc_parse("trace", "0Xg", p.hex, &r);
    EXPECT_FALSE(parsed);
    parser_deinit(&p);
}

TEST(ParserTest, TestDisassembly) {
    parser_t p = parser_init();
    mpc_result_t r;
    int parsed = mpc_parse("trace", "auipc sp,      16", p.disassembly, &r);
    EXPECT_TRUE(parsed);
    EXPECT_STREQ(((mpc_ast_t *)r.output)->contents, "auipc sp,      16");
    parser_deinit(&p);
}
