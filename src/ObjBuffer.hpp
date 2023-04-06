#ifndef OBJBUFFER_HPP
#define OBJBUFFER_HPP
#include <llvm/Object/ELF.h>
#include <llvm/Object/ELFTypes.h>
#include <llvm/Object/ObjectFile.h>

/*
 * Class to provide abstractions and utilities for working with the binary file. Maintains ownership
 * of the internal LLVM representation of an object file. Provides informations about the .text
 * section of the program.
 */
using llvm::object::ELF32LEFile;
class ObjBuffer {
    llvm::object::OwningBinary<llvm::object::ObjectFile> Obj;
    ELF32LEFile Elf;
    ELF32LEFile::Elf_Shdr TextShdr;

    llvm::object::OwningBinary<llvm::object::ObjectFile> initObj(const char *filename);
    ELF32LEFile initElf(llvm::StringRef ref, const char *filename);
    ELF32LEFile::Elf_Shdr initTextShdr(ELF32LEFile Elf);

  public:
    // Get the starting address of the .text section
    uint32_t getTextSectionStartingAddr();
    // Get the triple from the binary itself
    llvm::Triple getTriple();
    // const char *filename: path to binary file
    ObjBuffer(const char *filename);

    // Operator to access the internals of the binary as an ArrayRef
    operator llvm::ArrayRef<uint8_t>();

    ~ObjBuffer();
};

#endif
