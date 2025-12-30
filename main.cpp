#include "interpreter/lexer/lexer.h"
#include "interpreter/parser/parser.h"
#include "interpreter/interpreter/interpreter.h"

#ifdef BALD_HAS_VM
#include "compiler/compiler.h"
#include "vm/vm.h"
#endif

#ifdef BALD_HAS_OPTIMIZATIONS
#include "optimizer/const_folding.h"
#include "optimizer/dead_code.h"
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstring>

void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " [options] <source.bald>" << std::endl;
    std::cout << "\nOptions:" << std::endl;
#ifdef BALD_HAS_VM
    std::cout << "  --vm              Use bytecode VM (default)" << std::endl;
    std::cout << "  --interpreter     Use AST-walking interpreter" << std::endl;
    std::cout << "  --optimize, -O    Enable optimizations (constant folding, dead code elimination)" << std::endl;
    std::cout << "  --dump-bytecode   Show compiled bytecode" << std::endl;
#endif
    std::cout << "  --stats           Show execution statistics" << std::endl;
    std::cout << "  --help            Show this help" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    // Parse arguments
    bool use_vm = true;
    bool show_stats = false;
    bool optimize = false;
    bool dump_bytecode = false;
    const char* filename = nullptr;
    
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (std::strcmp(argv[i], "--interpreter") == 0) {
            use_vm = false;
        } else if (std::strcmp(argv[i], "--vm") == 0) {
            use_vm = true;
        } else if (std::strcmp(argv[i], "--stats") == 0) {
            show_stats = true;
        } else if (std::strcmp(argv[i], "--optimize") == 0 || std::strcmp(argv[i], "-O") == 0) {
            optimize = true;
        } else if (std::strcmp(argv[i], "--dump-bytecode") == 0) {
            dump_bytecode = true;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
        }
    }
    
    if (!filename) {
        std::cerr << "Error: No input file specified" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
#ifndef BALD_HAS_VM
    if (use_vm) {
        std::cerr << "Warning: VM not available in this build, using interpreter" << std::endl;
        use_vm = false;
    }
#endif
    
    try {
        // Read source file
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return 1;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Lexer
        Interpreter::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        // Parser
        Interpreter::Parser parser(tokens);
        auto ast = parser.parse();
        
        auto parse_time = std::chrono::high_resolution_clock::now();
        
#ifdef BALD_HAS_VM
        if (use_vm) {
            // VM path
            BaldVM::Compiler compiler;
            auto program = compiler.compile(ast);
            
            auto compile_time = std::chrono::high_resolution_clock::now();
            
            size_t before_opt = program.bytecode.size();
            size_t after_opt = before_opt;
            
            // Apply optimizations only if --optimize flag is set
#ifdef BALD_HAS_OPTIMIZATIONS
            if (optimize) {
                std::cout << "[Optimizer] Optimizations enabled" << std::endl;
                
                BaldVM::Optimizer::ConstantFoldingOptimizer const_fold;
                BaldVM::Optimizer::DeadCodeOptimizer dead_code;
                
                const_fold.optimize(program.bytecode, program.constants);
                dead_code.optimize(program.bytecode, program.constants);
                
                after_opt = program.bytecode.size();
            }
#else
            if (optimize) {
                std::cerr << "Warning: Optimizations not available in this build" << std::endl;
            }
#endif
            
            auto opt_time = std::chrono::high_resolution_clock::now();
            
            // Dump bytecode if requested
            if (dump_bytecode) {
                std::cout << "\n=== BYTECODE ";
                if (optimize) {
                    std::cout << "(OPTIMIZED)";
                } else {
                    std::cout << "(UNOPTIMIZED)";
                }
                std::cout << " ===" << std::endl;
                
                for (size_t i = 0; i < program.bytecode.size(); ++i) {
                    std::cout << "  " << i << ": " 
                             << BaldVM::opcodeToString(program.bytecode[i].opcode)
                             << " " << program.bytecode[i].operand << std::endl;
                }
                std::cout << "===================\n" << std::endl;
            }
            
            // Execute on VM
            BaldVM::VirtualMachine vm;
            vm.execute(program.bytecode, program.constants, 
                      program.variable_names, std::cout);
            
            auto end_time = std::chrono::high_resolution_clock::now();
            
            if (show_stats) {
                std::cout << "\n=== VM Statistics ===" << std::endl;
                std::cout << " Mode: Bytecode VM" << std::endl;
#ifdef BALD_HAS_OPTIMIZATIONS
                if (optimize) {
                    std::cout << " Optimizations: ENABLED" << std::endl;
                    std::cout << " Bytecode instructions: " << before_opt 
                             << " → " << after_opt 
                             << " (removed: " << (before_opt - after_opt) << ")" << std::endl;
                } else {
                    std::cout << " Optimizations: DISABLED" << std::endl;
                    std::cout << " Bytecode instructions: " << before_opt << std::endl;
                }
#else
                std::cout << " Bytecode instructions: " << before_opt << std::endl;
#endif
                std::cout << " Instructions executed: " << vm.getInstructionsExecuted() << std::endl;
                std::cout << " Stack peak: " << vm.getStackPeak() << std::endl;
#ifdef BALD_HAS_GC
                std::cout << " GC collections: " << vm.getGCCollections() << std::endl;
#endif
                
                auto parse_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                    parse_time - start_time).count() / 1000.0;
                auto compile_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                    compile_time - parse_time).count() / 1000.0;
                auto opt_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                    opt_time - compile_time).count() / 1000.0;
                auto exec_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                    end_time - opt_time).count() / 1000.0;
                
                std::cout << "\n=== Timing ===" << std::endl;
                std::cout << " Parsing:      " << parse_ms << " ms" << std::endl;
                std::cout << " Compilation:  " << compile_ms << " ms" << std::endl;
                std::cout << " Optimization: " << opt_ms << " ms" << std::endl;
                std::cout << " Execution:    " << exec_ms << " ms" << std::endl;
                std::cout << " Total:        " << (parse_ms + compile_ms + opt_ms + exec_ms) << " ms" << std::endl;
            }
        } else
#endif
        {
            // AST interpreter path
            std::istringstream input_stream(source);
            Interpreter::interpret(input_stream, std::cout);           
            auto end_time = std::chrono::high_resolution_clock::now();
            
            if (show_stats) {
                auto parse_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                    parse_time - start_time).count() / 1000.0;
                auto exec_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                    end_time - parse_time).count() / 1000.0;
                
                std::cout << "\n=== Interpreter Statistics ===" << std::endl;
                std::cout << " Mode: AST-walking" << std::endl;
                std::cout << "\n=== Timing ===" << std::endl;
                std::cout << " Parsing:   " << parse_ms << " ms" << std::endl;
                std::cout << " Execution: " << exec_ms << " ms" << std::endl;
                std::cout << " Total:     " << (parse_ms + exec_ms) << " ms" << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

