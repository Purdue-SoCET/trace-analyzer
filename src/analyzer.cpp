#include "analyzer.hpp"
#define GET_INSTRINFO_ENUM
#include <lib/Target/RISCV/RISCVGenInstrInfo.inc>
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
    : Obj(filename), stats({0}) {
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
        uint32_t opcode = I.getOpcode();
        switch (opcode) {
        // RV32I ALU
        case llvm::RISCV::LUI:
        case llvm::RISCV::AUIPC:
        case llvm::RISCV::ADDI:
        case llvm::RISCV::SLTI:
        case llvm::RISCV::SLTIU:
        case llvm::RISCV::XORI:
        case llvm::RISCV::ORI:
        case llvm::RISCV::ANDI:
        case llvm::RISCV::SLLI:
        case llvm::RISCV::SRLI:
        case llvm::RISCV::SRAI:
        case llvm::RISCV::ADD:
        case llvm::RISCV::SUB:
        case llvm::RISCV::SLL:
        case llvm::RISCV::SLT:
        case llvm::RISCV::SLTU:
        case llvm::RISCV::XOR:
        case llvm::RISCV::SRL:
        case llvm::RISCV::SRA:
        case llvm::RISCV::OR:
        case llvm::RISCV::AND:
        // RV32C ALU
        case llvm::RISCV::C_ADDI4SPN:
        case llvm::RISCV::C_ADDI:
        case llvm::RISCV::C_LI:
        case llvm::RISCV::C_ADDI16SP:
        case llvm::RISCV::C_LUI:
        case llvm::RISCV::C_SRLI:
        case llvm::RISCV::C_SRAI:
        case llvm::RISCV::C_ANDI:
        case llvm::RISCV::C_SUB:
        case llvm::RISCV::C_XOR:
        case llvm::RISCV::C_OR:
        case llvm::RISCV::C_AND:
        case llvm::RISCV::C_SLLI:
        case llvm::RISCV::C_MV:
        case llvm::RISCV::C_ADD:
            this->stats.alu++;
            break;
        // RV32M Multiply Divide
        case llvm::RISCV::MUL:
        case llvm::RISCV::MULH:
        case llvm::RISCV::MULHSU:
        case llvm::RISCV::MULHU:
        case llvm::RISCV::DIV:
        case llvm::RISCV::DIVU:
        case llvm::RISCV::REM:
        case llvm::RISCV::REMU:
            this->stats.muldiv++;
            break;
        // RV32I Branch
        case llvm::RISCV::BEQ:
        case llvm::RISCV::BNE:
        case llvm::RISCV::BLT:
        case llvm::RISCV::BGE:
        case llvm::RISCV::BLTU:
        case llvm::RISCV::BGEU:
        // RV32C Branch
        case llvm::RISCV::C_BEQZ:
        case llvm::RISCV::C_BNEZ:
            this->stats.branch++;
            break;
        // RV32I Call
        case llvm::RISCV::JAL:
        case llvm::RISCV::JALR:
        case llvm::RISCV::C_JAL:
        case llvm::RISCV::C_JALR:
        case llvm::RISCV::C_J:
        case llvm::RISCV::C_JR:
            this->stats.call++;
            break;
        // RV32I Mem
        case llvm::RISCV::LB:
        case llvm::RISCV::LH:
        case llvm::RISCV::LW:
        case llvm::RISCV::LBU:
        case llvm::RISCV::LHU:
        case llvm::RISCV::LWU:
        case llvm::RISCV::SB:
        case llvm::RISCV::SH:
        case llvm::RISCV::SW:
        // RV32C Mem
        case llvm::RISCV::C_LW:
        case llvm::RISCV::C_SW:
        case llvm::RISCV::C_LWSP:
        case llvm::RISCV::C_SWSP:
            this->stats.mem++;
            break;
        // RV32I System
        case llvm::RISCV::FENCE:
        case llvm::RISCV::FENCE_I:
        case llvm::RISCV::CSRRW:
        case llvm::RISCV::CSRRS:
        case llvm::RISCV::CSRRC:
        case llvm::RISCV::CSRRWI:
        case llvm::RISCV::CSRRSI:
        case llvm::RISCV::CSRRCI:
        case llvm::RISCV::ECALL:
        case llvm::RISCV::EBREAK:
        case llvm::RISCV::URET:
        case llvm::RISCV::SRET:
        case llvm::RISCV::MRET:
        case llvm::RISCV::WFI:
        case llvm::RISCV::SFENCE_VMA:
        // RV32C System
        case llvm::RISCV::C_NOP:
        case llvm::RISCV::C_EBREAK:
            this->stats.system++;
            break;
        default:
            printf("Unknown instruction: %s (Opcode %d)\n",
                   info->getName(I.getOpcode()).data(), opcode);
            break;
        }
    }

    return true;
}

void Analyzer::displayStatistics() {
    auto alu = this->stats.alu;
    auto mem = this->stats.mem;
    auto branch = this->stats.branch;
    auto call = this->stats.call;
    auto muldiv = this->stats.muldiv;
    auto system = this->stats.system;
    float total =
        std::max<std::size_t>(1, alu + mem + branch + call + muldiv + system);
    printf("Statistics\n"
           "=========================\n"
           "Type         Count      %%\n"
           "=========================\n"
           "ALU: %12lu  %5.2f%%\n"
           "Mem: %12lu  %5.2f%%\n"
           "Branch: %9lu  %5.2f%%\n"
           "Call: %11lu  %5.2f%%\n"
           "Mul/Div: %8lu  %5.2f%%\n"
           "System: %9lu  %5.2f%%\n",
           alu, 100 * alu / total, mem, 100 * mem / total, branch,
           100 * branch / total, call, 100 * call / total, muldiv,
           100 * muldiv / total, system, 100 * system / total);
}

void Analyzer::displayStatisticsJson() {
    auto alu = this->stats.alu;
    auto mem = this->stats.mem;
    auto branch = this->stats.branch;
    auto call = this->stats.call;
    auto muldiv = this->stats.muldiv;
    auto system = this->stats.system;
    auto total = alu + mem + branch + call + muldiv + system;
    printf("{\n"
           "    \"count\": {\n"
           "        \"alu\": %lu,\n"
           "        \"mem\": %lu,\n"
           "        \"branch\": %lu,\n"
           "        \"call\": %lu,\n"
           "        \"muldiv\": %lu,\n"
           "        \"system\": %lu\n"
           "    }\n"
           "    \"total\": %lu\n"
           "}\n",
           alu, mem, branch, call, muldiv, system, total);
}
