#ifndef ANALYZER_HPP
#define ANALYZER_HPP
#include "ObjBuffer.hpp"
#include <llvm/MC/MCInst.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <string>
#include <vector>

class Analyzer {
    typedef struct {
        uint32_t hex;
        uint32_t pc;
    } Instruction;
    std::vector<Instruction> instructions;
    ObjBuffer Obj;

  public:
    std::vector<llvm::MCInst> disassembly;
    typedef struct {
        std::size_t alu;
        std::size_t mem;
        std::size_t branch;
        std::size_t call;
        std::size_t muldiv;
        std::size_t system;
    } statistics;
    statistics stats;

    // Initialize an analyzer with a set of instructions and PC values
    // int: initial capacity of members, defaults to 0
    // std::vector<std::string> instr: instruction hex to be added to internal
    // list of instructions
    // std::vector<std::string> pc: pc hex to be added to internal list of pcs
    Analyzer(std::vector<std::string> instr, std::vector<std::string> pc,
             const char *filename);
    // Prints statistics as a table
    void displayStatistics();
    // Prints statistics as a JSON object
    void displayStatisticsJson();
    // Prints statistics as a MATLAB array
    void displayStatisticsMatlab();
    // Analyze all instructions
    bool analyze();
    ~Analyzer() = default;
};
#endif
