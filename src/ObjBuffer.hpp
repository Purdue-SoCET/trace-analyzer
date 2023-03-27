#ifndef OBJBUFFER_HPP
#define OBJBUFFER_HPP
#include <llvm/Object/ELF.h>
#include <llvm/Object/ELFTypes.h>
#include <llvm/Object/ObjectFile.h>

using llvm::object::ELF32LEFile;
class ObjBuffer {
    llvm::object::OwningBinary<llvm::object::ObjectFile> Obj;
    ELF32LEFile Elf;
    ELF32LEFile::Elf_Shdr TextShdr;

    llvm::object::OwningBinary<llvm::object::ObjectFile>
    initObj(const char *filename);
    ELF32LEFile initElf(llvm::StringRef ref, const char *filename);
    ELF32LEFile::Elf_Shdr initTextShdr(ELF32LEFile Elf);

  public:
    uint32_t getTextSectionStartingAddr();
    llvm::Triple getTriple();
    ObjBuffer(const char *filename);

    operator llvm::ArrayRef<uint8_t>();

    ~ObjBuffer();
};

#endif
