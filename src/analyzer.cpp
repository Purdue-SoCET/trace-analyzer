#include "analyzer.hpp"
#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCDisassembler/MCDisassembler.h>
#include <llvm/MC/MCInstrDesc.h>
#include <llvm/MC/MCInstrInfo.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/MC/SubtargetFeature.h>
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

    LLVMInitializeRISCVTarget();
    LLVMInitializeRISCVTargetInfo();
    LLVMInitializeRISCVTargetMC();
    LLVMInitializeRISCVAsmParser();
    LLVMInitializeRISCVAsmPrinter();
    LLVMInitializeRISCVDisassembler();
}

bool Analyzer::analyze() {
    llvm::Triple triple = this->Obj.getTriple();
    std::string error;
    const llvm::Target *target =
        llvm::TargetRegistry::lookupTarget("riscv32", triple, error);
    if (!target) {
        fprintf(stderr, "Failed to create target: %s\n", error.c_str());
        return false;
    }

    // Set up LLVM infrastructure for diassembly
    // TODO: make this more robust
    llvm::SubtargetFeatures stf;
    for (auto F : std::string("mvc")) {
        stf.AddFeature(llvm::StringRef(&F, 1));
    }
    std::unique_ptr<llvm::MCSubtargetInfo> sti(
        target->createMCSubtargetInfo(triple.getTriple(), "", stf.getString()));
    std::unique_ptr<llvm::MCRegisterInfo> mri(
        target->createMCRegInfo(triple.getTriple()));
    std::unique_ptr<llvm::MCAsmInfo> mai(target->createMCAsmInfo(
        *mri, triple.getTriple(), llvm::MCTargetOptions()));
    llvm::MCContext ctx(triple, mai.get(), mri.get(), sti.get());
    std::unique_ptr<llvm::MCDisassembler> disasm(
        target->createMCDisassembler(*sti, ctx));

    // Disassemble by PC
    auto bytes = static_cast<llvm::ArrayRef<uint8_t>>(this->Obj);
    for (auto I : this->instructions) {
        uint32_t pc = I.pc;
        llvm::MCInst Inst;
        uint64_t size = 0;
        auto len = (pc + 4 > bytes.size()) ? bytes.size() - pc : 4;
        switch (disasm->getInstruction(Inst, size, bytes.slice(pc, len), pc,
                                       llvm::nulls())) {
        case llvm::MCDisassembler::SoftFail:
        case llvm::MCDisassembler::Fail:
            fprintf(stderr, "MCDisassembler failed!\n");
            return false;
        case llvm::MCDisassembler::Success:
            this->disassembly.push_back(Inst);
            break;
        }
    }

    std::unique_ptr<llvm::MCInstrInfo> info(target->createMCInstrInfo());
    for (auto I : this->disassembly) {
        llvm::MCInstrDesc desc = info->get(I.getOpcode());
        (void)desc;
    }

    return true;
}
