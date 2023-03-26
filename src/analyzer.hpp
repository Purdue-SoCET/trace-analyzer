#ifndef ANALYZER_HPP
#define ANALYZER_HPP
#include <llvm/MC/MCInst.h>
#include <string>
#include <vector>

class Analyzer {
    typedef struct Instruction {
        uint32_t hex;
        uint32_t pc;
        uint8_t size;
    } Instruction;
    std::vector<Instruction> instructions;
    std::vector<uint8_t> bytes;

  public:
    std::vector<llvm::MCInst> disassembly;

    // Initialize an analyzer with a set of instructions and PC values
    // int: initial capacity of members, defaults to 0
    // std::vector<std::string> instr: instruction hex to be added to internal
    // list of instructions
    // std::vector<std::string> pc: pc hex to be added to internal list of pcs
    Analyzer(std::vector<std::string> instr, std::vector<std::string> pc);
    // Analyze all instructions
    bool analyze();
    ~Analyzer() = default;
};
#endif
