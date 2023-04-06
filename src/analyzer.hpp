#ifndef ANALYZER_HPP
#define ANALYZER_HPP
#include "ObjBuffer.hpp"
#include <llvm/MC/MCInst.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <string>
#include <vector>

/*
 * This is the main class that performs the analysis on the instructions. It uses the original ELF
 * file used to generate the trace to provide a reliable source of instructions.
 */
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
    // std::vector<std::string> instr: instruction to be parsed into an internal Instruction type
    // std::vector<std::string> pc: program counter associated with the instruction
    // const char *filename: the path to the binary file used to generate
    Analyzer(std::vector<std::string> instr, std::vector<std::string> pc, const char *filename);
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
