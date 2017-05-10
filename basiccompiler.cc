#include <iostream>
#include <fstream>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include "lexer.h"
#include "parser.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Usage: basiccompiler INPUTFILE OUTPUTFILE\n";
        return 1;
    }

    std::ifstream input_file(argv[1]);
    BASICLexer lexer;
    if (!lexer.readFromStream(input_file)) return 1;

    BASICParser parser;
    if (!parser.parseFromTokenList(lexer.getTokens())) return 1;
    llvm::Module *mod = parser.generateModule();
    if (mod == nullptr) return 1;

    std::error_code e;
    llvm::raw_fd_ostream output_file(argv[2], e, llvm::sys::fs::OpenFlags::F_None);
    llvm::WriteBitcodeToFile(mod, output_file);

    return 0;
}
