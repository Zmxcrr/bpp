#include "interpreter.h"
#include "gc.h"
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Bald++ Interpreter v1.0" << std::endl;
        std::cout << "Usage: " << argv[0] << " <program.bald>" << std::endl;
        std::cout << std::endl;
        std::cout << "Example programs:" << std::endl;
        std::cout << "  " << argv[0] << " factorial_test.bald" << std::endl;
        std::cout << "  " << argv[0] << " hello_world.bald" << std::endl;
        std::cout << "  " << argv[0] << " comprehensive_benchmark.bald" << std::endl;
        std::cout << std::endl;
        std::cout << "Note: Bald++ programs use the .bald file extension" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    if (filename.length() < 5 || filename.substr(filename.length() - 5) != ".bald") {
        std::cerr << "Warning: Expected .bald file extension for Bald++ programs" << std::endl;
    }

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();

    Interpreter::GC::Collector::instance().setThreshold(100);

    try {
        Interpreter::Lexer lexer(source);
        auto tokens = lexer.tokenize();

        bool lexerError = false;
        for (const auto& token : tokens) {
            if (token.type == Interpreter::TokenType::Unknown) {
                std::cerr << "Line " << token.line << ", Column " << token.column 
                          << ": Unknown token '" << token.text << "'" << std::endl;
                lexerError = true;
            }
        }

        if (lexerError) {
            return 1;
        }

        Interpreter::Parser parser(tokens);
        auto statements = parser.parse();

        if (parser.parsingFailed()) {
            std::cerr << "Parsing failed." << std::endl;
            return 1;
        }

        auto* global_gc = Interpreter::GC::allocate<Interpreter::GC::GCEnvironment>(
            new Interpreter::Environment()
        );
        
        Interpreter::GC::ScopedRoot root(global_gc);

        for (const auto& stmt : statements) {
            if (!stmt) {
                std::cerr << "Null statement encountered." << std::endl;
                return 1;
            }
            stmt->execute(global_gc->env, std::cout);
        }

        Interpreter::GC::collect();
        
        auto stats = Interpreter::GC::getStats();
        if (stats.collections_count > 0) {
            std::cerr << "\n[GC Statistics]" << std::endl;
            std::cerr << "  Collections: " << stats.collections_count << std::endl;
            std::cerr << "  Objects collected: " << stats.objects_collected << std::endl;
            std::cerr << "  Objects alive: " << stats.total_objects << std::endl;
            std::cerr << "  Root objects: " << stats.roots_count << std::endl;
        }

    } catch (const Interpreter::ReturnException& e) {
        return 0;
    } catch (const Interpreter::BreakException& e) {
        std::cerr << "Break statement outside loop." << std::endl;
        return 1;
    } catch (const Interpreter::ContinueException& e) {
        std::cerr << "Continue statement outside loop." << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

