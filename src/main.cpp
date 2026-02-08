#include "parser.hpp"
#include "lexer.hpp"
#include "semanticAnalyzer.hpp"
#include "symbolTable.hpp"
#include "IRBuilder.hpp"
#include "IR.hpp"
#include "assemblyEmitter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

fs::path getExePath() {
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    return fs::path(buffer).parent_path();
#else
    return fs::canonical("/proc/self/exe").parent_path();
#endif
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: dyc <input.dy>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    if(!input.is_open()) {
        std::cerr << "Failed to open: " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << input.rdbuf();
    std::string source = buffer.str();
    input.close();

    std::string inputName = argv[1];
    std::string outputName;
    size_t dot = inputName.rfind('.');
    if(dot != std::string::npos) {
        outputName = inputName.substr(0, dot) + ".dvs";
    } else {
        outputName = inputName + ".dvs";
    }

    try {
        Lexer lexer(source);
        std::vector<TokenData> tokens = lexer.tokenize();
        Parser parser(tokens);
        Program* program = parser.parse();

        std::vector<FunctionDecl*> allFunctions = program->getFunctions();

        fs::path dylibPath = getExePath().parent_path() / "dylib";

        for(ImportDecl* imp : program->getImports()) {
            fs::path modulePath = dylibPath / (imp->getModuleName() + ".dy");

            std::ifstream moduleFile(modulePath.string());
            if(!moduleFile.is_open()) {
                std::cerr << "Failed to open module: " << modulePath.string() << std::endl;
                return 1;
            }

            std::stringstream moduleBuffer;
            moduleBuffer << moduleFile.rdbuf();
            std::string moduleSource = moduleBuffer.str();
            moduleFile.close();

            Lexer moduleLexer(moduleSource);
            std::vector<TokenData> moduleTokens = moduleLexer.tokenize();
            Parser moduleParser(moduleTokens);
            Program* moduleProgram = moduleParser.parse();

            for(FunctionDecl* func : moduleProgram->getFunctions()) {
                if(imp->isSelectiveImport()) {
                    for(const std::string& sym : imp->getSymbols()) {
                        if(func->getName() == sym) {
                            allFunctions.push_back(func);
                            break;
                        }
                    }
                } else {
                    allFunctions.push_back(func);
                }
            }
        }

        Program* mergedProgram = new Program({}, allFunctions, 0, 0);

        SymbolTable symTable;
        SemanticAnalyzer analyzer(&symTable);

        if(!analyzer.analyze(mergedProgram)) {
            std::cerr << "Semantic Analysis Failed:" << std::endl;
            analyzer.print_errors();
            return 1;
        }

        IRModule* module = new IRModule();
        IRBuilder builder(module, &symTable);
        builder.visitProgram(mergedProgram);

        std::ofstream outFile(outputName);
        AssemblyEmitter emitter(outFile);
        emitter.emit(module);
        outFile.close();

        std::cout << "Compiled: " << outputName << std::endl;

        delete module;
        delete mergedProgram;
        delete program;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
