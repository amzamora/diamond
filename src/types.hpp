#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <vector>
#include <memory>

// Source file
struct Source {
	size_t line;
	size_t col;
	std::string file;
	std::string::iterator it;
	std::string::iterator end;

	Source() {}
	Source(std::string file, std::string::iterator it, std::string::iterator end) : line(1), col(1), file(file), it(it), end(end) {}
};
char current(Source source);
bool at_end(Source source);
bool match(Source source, std::string to_match);
Source operator+(Source source, size_t offset);

// Parser result
template <class T>
struct ParserResult {
	T value;
	Source source;
	std::string error_message;

	bool error() {
		if (this->error_message == "") return false;
		else                           return true;
	}

	ParserResult<T>(T value, Source source, std::string error_message = "") : value(value), source(source), error_message(error_message) {}
	ParserResult<T>(Source source, std::string error_message) : source(source), error_message(error_message) {}
};

// Ast
namespace Ast {
	struct Program;
	struct Number;

	struct Visitor {
		virtual void visit(Program* node) = 0;
		virtual void visit(Number* node) = 0;
		virtual ~Visitor() {}
	};

	struct Node {
		size_t line;
		size_t col;
		std::string file;

		Node(size_t line, size_t col, std::string file): line(line), col(col), file(file) {}
		virtual ~Node() {}
		virtual void print(size_t indent_level = 0) = 0;
		virtual void accept(Visitor* visitor) = 0;
	};

	struct Program : Node {
		std::vector<Ast::Node*> expressions;

		Program(std::vector<Ast::Node*> expressions, size_t line, size_t col, std::string file) : Node(line, col, file), expressions(expressions) {}
		virtual ~Program();
		virtual void print(size_t indent_level = 0);
		virtual void accept(Visitor* visitor) override {
			visitor->visit(this);
		}
	};

	struct Number : Node {
		double value;

		Number(double value, size_t line, size_t col, std::string file) : Node(line, col, file), value(value) {}
		virtual ~Number();
		virtual void print(size_t indent_level = 0);
		virtual void accept(Visitor* visitor) override {
			visitor->visit(this);
		}
	};
}

#endif
