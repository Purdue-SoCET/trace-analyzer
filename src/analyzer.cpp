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

Analyzer::Analyzer(uint32_t n) {
    this->instructions.reserve(n);
    this->disassembly.reserve(n);
}

void Analyzer::add_instrs(std::vector<std::string> instr,
                          std::vector<std::string> pc) {
    assert(instr.size() == pc.size());
    std::vector<uint32_t> instrs{};
    instrs.reserve(instr.size());
    for (auto hex : instr) {
        instrs.push_back(std::stoul(hex, nullptr, 16));
    }
    std::vector<uint32_t> pcs{};
    pcs.reserve(pcs.size());
    for (auto pc : pc) {
        pcs.push_back(std::stoul(pc, nullptr, 16));
    }

    for (uintptr_t i = 0; i < instr.size(); i++) {
        Instruction I;
        I.hex = instrs[i];
        I.pc = pcs[i];
        I.size = i == (instrs.size() - 1) ? 4 : pcs[i + 1] - pcs[i];
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
