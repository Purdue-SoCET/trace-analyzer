#include "ObjBuffer.hpp"
#include <llvm/ADT/StringRef.h>
#include <llvm/Object/Decompressor.h>
#include <llvm/Object/ELFObjectFile.h>

ObjBuffer::ObjBuffer(const char *filename)
    : Obj(initObj(filename)),
      Elf(initElf(Obj.getBinary()->getMemoryBufferRef().getBuffer(), filename)),
      TextShdr(initTextShdr(Elf)) {}

llvm::object::OwningBinary<llvm::object::ObjectFile>
ObjBuffer::initObj(const char *filename) {
    auto ObjOrErr = llvm::object::ObjectFile::createObjectFile(filename);
    if (ObjOrErr.takeError()) {
        fprintf(stderr, "Failed to create object file from %s!\n", filename);
        exit(EXIT_FAILURE);
    }
    return std::move(*ObjOrErr);
}

llvm::object::ELF32LEFile ObjBuffer::initElf(llvm::StringRef ref,
                                             const char *filename) {
    auto ElfOrErr = llvm::object::ELF32LEFile::create(ref);
    if (ElfOrErr.takeError()) {
        fprintf(stderr, "LLVM failed to create an ELF file from %s\n",
                filename);
        exit(EXIT_FAILURE);
    }
    return *ElfOrErr;
}

llvm::object::ELF32LEFile::Elf_Shdr ObjBuffer::initTextShdr(ELF32LEFile Elf) {
    auto SectionsOrErr = this->Elf.sections();
    if (SectionsOrErr.takeError()) {
        fprintf(stderr, "Failed to get sections from ELF!\n");
        exit(EXIT_FAILURE);
    }
    auto Sections = *SectionsOrErr;
    auto SectionStringTableOrErr = this->Elf.getSectionStringTable(Sections);
    if (SectionStringTableOrErr.takeError()) {
        fprintf(stderr, "Failed to get section string table!\n");
        exit(EXIT_FAILURE);
    }
    auto SectionStringTable = (*SectionStringTableOrErr);
    auto TextOrNone =
        std::find_if(Sections.begin(), Sections.end(), [&](auto Shdr) {
            llvm::ELF::Elf32_Word sh_name = Shdr.sh_name;
            return strcmp(SectionStringTable.data() + sh_name, ".text") == 0;
        });
    if (TextOrNone == Sections.end()) {
        fprintf(stderr, "Failed to find .text section!");
        exit(EXIT_FAILURE);
    }
    auto Text = *TextOrNone;
    return Text;
}

llvm::Triple ObjBuffer::getTriple() {
    return this->Obj.getBinary()->makeTriple();
}

uint32_t ObjBuffer::getTextSectionStartingAddr() {
    return this->TextShdr.sh_addr;
}

ObjBuffer::operator llvm::ArrayRef<uint8_t>() {
    auto SectionDataOrErr = this->Elf.getSectionContents(this->TextShdr);
    if (SectionDataOrErr.takeError()) {
        fprintf(stderr, "Failed to get section data!\n");
        exit(EXIT_FAILURE);
    }
    auto SectionData = *SectionDataOrErr;
    return SectionData;
}

ObjBuffer::~ObjBuffer() {}
