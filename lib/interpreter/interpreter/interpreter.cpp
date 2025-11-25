#include "interpreter.h"
#include "parser.h"
#include "lexer.h"
#include "exception.h"
#include <iostream>

namespace Interpreter {
	std::istream* CurrentIn  = &std::cin;
	std::ostream* CurrentOut = &std::cout;

	bool interpret(std::istream &in, std::ostream &out) {
		std::string code((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
		if (code.empty()) return true;
		try {
				static std::istringstream kEmptyRuntimeInput;
				CurrentIn  = &kEmptyRuntimeInput;
				CurrentOut = &out;
		    Lexer lexer(code);
		    auto tokens = lexer.tokenize();
		    bool lexerError = false;
		    for (const auto &token: tokens) {
			if (token.type == TokenType::Unknown) {
			    std::cerr << "[Строка " << token.line << ", столбец " << token.column
				    << "] Ошибка лексера: Неизвестный символ или последовательность '"
				    << token.text << "'" << std::endl;
			    lexerError = true;
			}
		    }
		    if (lexerError) return false;
		    Parser parser(tokens);
		    auto program = parser.parse();
		    if (parser.parsingFailed()) {
			out << "Ошибка синтаксиса: обнаружены ошибки при разборе программы." << std::endl;
			return false;
		    }
		    Environment global;
		    for (const auto &stmt: program) {
			if (!stmt) {
			    std::cerr << "Ошибка выполнения: Обнаружен null statement." << std::endl;
			    return false;
			}
			stmt->execute(&global, out);
		    }
		    return true;
		} catch (const ReturnException &) {
		    std::cerr << "Ошибка выполнения: 'return' вне функции." << std::endl;
		    return false;
		}
		catch (const BreakException &) {
		    std::cerr << "Ошибка выполнения: 'break' вне цикла." << std::endl;
		    return false;
		}
		catch (const ContinueException &) {
		    std::cerr << "Ошибка выполнения: 'continue' вне цикла." << std::endl;
		    return false;
		}
		catch (const std::runtime_error &e) {
		    const std::string err = e.what();
		    if (err != "Шаг для функции 'range' не может быть равен 0")
			out << "Ошибка выполнения: " << err << std::endl;
		    return false;
		}
		catch (const std::exception &e) {
		    out << "Системная ошибка: " << e.what() << std::endl;
		    return false;
		}
		catch (...) {
		    out << "Неизвестная ошибка выполнения." << std::endl;
		    return false;
		}
	}
} //Interpreter