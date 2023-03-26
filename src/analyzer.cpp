#include "analyzer.hpp"
#include <algorithm>
#include <execution>
#include <iostream>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCDisassembler/MCDisassembler.h>
#include <llvm/MC/MCInstrDesc.h>
#include <llvm/MC/MCInstrInfo.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

Analyzer::Analyzer(std::vector<std::string> instrs,
                   std::vector<std::string> pcs) {
    assert(instrs.size() == pcs.size());
    auto n = instrs.size();
    this->instructions.reserve(n);
    this->disassembly.reserve(n);

    // Add instructions to internal storage
    for (uintptr_t i = 0; i < instrs.size(); i++) {
        uint32_t instr = std::stoul(instrs[i], nullptr, 16);
        uint32_t curr_pc = std::stoul(pcs[i], nullptr, 16);
        uint32_t next_pc =
            i == (instrs.size() - 1) ? 0 : std::stoul(pcs[i + 1], nullptr, 16);
        Instruction I;
        I.hex = instr;
        I.pc = curr_pc;
        I.size = i == (instrs.size() - 1) ? 4 : next_pc - curr_pc;
        this->instructions.push_back(I);
    }

    for (auto I : this->instructions) {
        if (I.size == 4) {
            this->bytes.push_back(I.hex);
        } else {
            this->bytes.push_back(static_cast<uint16_t>(I.hex));
        }
    }
}

bool Analyzer::analyze() {
    LLVMInitializeRISCVTargetInfo();
    LLVMInitializeRISCVTargetMC();
    LLVMInitializeRISCVAsmParser();
    LLVMInitializeRISCVAsmPrinter();
    LLVMInitializeRISCVDisassembler();

    llvm::Triple triple("riscv32-unknown-none");
    llvm::MCSubtargetInfo sti(
        triple, "unknown", "", "", llvm::ArrayRef<llvm::SubtargetFeatureKV>(),
        llvm::ArrayRef<llvm::SubtargetSubTypeKV>(), nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr);
    llvm::MCContext ctx(triple, nullptr, nullptr, nullptr);
    std::string error;
    const llvm::Target *target =
        llvm::TargetRegistry::lookupTarget("riscv32", triple, error);
    const llvm::MCDisassembler *disasm(target->createMCDisassembler(sti, ctx));

    for (auto I : this->instructions) {
        llvm::MCInst instr;
        uint64_t size = 0;
        // todo this->bytes wrong
        switch (disasm->getInstruction(instr, size, this->bytes, I.pc,
                                       llvm::nulls())) {
        case llvm::MCDisassembler::SoftFail:
        case llvm::MCDisassembler::Fail:
            printf("MCDisassembler failed!\n");
            return false;
        case llvm::MCDisassembler::Success:
            std::cout << "Successful disassembly!\n";
            this->disassembly.push_back(instr);
            break;
        }
    }

    llvm::MCInstrInfo *info = target->createMCInstrInfo();
    for (auto I : this->disassembly) {
        llvm::MCInstrDesc desc = info->get(I.getOpcode());
        // TODO:
        (void)desc;
    }

    return true;
}
