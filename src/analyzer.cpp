#include "analyzer.hpp"
#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCDisassembler/MCDisassembler.h>
#include <llvm/MC/MCInstrDesc.h>
#include <llvm/MC/MCInstrInfo.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

Analyzer::Analyzer(std::vector<std::string> instrs,
                   std::vector<std::string> pcs, const char *filename)
    : Obj(filename) {
    assert(instrs.size() == pcs.size());
    auto n = instrs.size();
    disassembly.reserve(n);

    uint32_t starting_addr = Obj.getTextSectionStartingAddr();
    // Add instructions to internal storage
    for (uintptr_t i = 0; i < instrs.size(); i++) {
        uint32_t instr = std::stoul(instrs[i], nullptr, 16);
        uint32_t curr_pc = std::stoul(pcs[i], nullptr, 16);
        Instruction I;
        I.hex = instr;
        I.pc = curr_pc - starting_addr;
        instructions.push_back(I);
    }
}

bool Analyzer::analyze() {
    LLVMInitializeRISCVTarget();
    LLVMInitializeRISCVTargetInfo();
    LLVMInitializeRISCVTargetMC();
    LLVMInitializeRISCVAsmParser();
    LLVMInitializeRISCVAsmPrinter();
    LLVMInitializeRISCVDisassembler();

    llvm::Triple triple = this->Obj.getTriple();
    std::string error;
    const llvm::Target *target =
        llvm::TargetRegistry::lookupTarget("riscv32", triple, error);
    if (!target) {
        fprintf(stderr, "Failed to create target: %s\n", error.c_str());
        return false;
    }
    std::unique_ptr<llvm::MCSubtargetInfo> sti(
        target->createMCSubtargetInfo(triple.getTriple(), "", ""));
    llvm::MCContext ctx(triple, nullptr, nullptr, nullptr);
    const llvm::MCDisassembler *disasm =
        target->createMCDisassembler(*sti, ctx);
    auto bytes = static_cast<llvm::ArrayRef<uint8_t>>(this->Obj);
    for (auto I : this->instructions) {
        uint32_t pc = I.pc;
        llvm::MCInst instr;
        uint64_t size = 0;
        switch (disasm->getInstruction(instr, size, bytes.slice(pc, 4), pc,
                                       llvm::nulls())) {
        case llvm::MCDisassembler::SoftFail:
        case llvm::MCDisassembler::Fail:
            fprintf(stderr, "MCDisassembler failed!\n");
            return false;
        case llvm::MCDisassembler::Success:
            this->disassembly.push_back(instr);
            break;
        }
    }

    llvm::MCInstrInfo *info = target->createMCInstrInfo();
    for (auto I : this->disassembly) {
        llvm::MCInstrDesc desc = info->get(I.getOpcode());
        (void)desc;
    }

    return true;
}
