#include <iostream>

#include "types.hpp"

// Source
char current(Source source) {
	return *(source.it);
}

bool at_end(Source source) {
	if (source.it >= source.end) return true;
	else                         return false;
}

bool match(Source source, std::string to_match) {
	for (size_t i = 0; i < to_match.size(); i++) {
		if (current(source) != to_match[i]) return false;
		source = source + 1;
	}
	return true;
}

Source addOne(Source source) {
	source.it++;
	if (current(source) == '\n') {
		source.line += 1;
		source.col = 1;
	}
	else {
		source.col++;
	}
	return source;
}

Source operator+(Source source, size_t offset) {
	if (offset == 0) return source;
	else             return addOne(source) + (offset - 1);
}

// Ast
// ---
void put_indent_level(size_t indent_level) {
	for (size_t i = 0; i < indent_level; i++) std::cout << "    ";
}

// Program
void Ast::Program::print(size_t indent_level) {
	put_indent_level(indent_level);
	std::cout << "program" << '\n';
	for (size_t i = 0; i < this->statements.size(); i++) {
		this->statements[i]->print(indent_level + 1);
	}
}

Ast::Program::~Program() {
	for (size_t i = 0; i < this->statements.size(); i++) {
		delete this->statements[i];
	}
}

// Call
void Ast::Call::print(size_t indent_level) {
	put_indent_level(indent_level);
	std::cout << this->identifier << '\n';
	for (size_t i = 0; i < this->args.size(); i++) {
		this->args[i]->print(indent_level + 1);
	}
}

Ast::Call::~Call() {
	for (size_t i = 0; i < this->args.size(); i++) {
		delete this->args[i];
	}
}

// Assignment
void Ast::Assignment::print(size_t indent_level) {
	put_indent_level(indent_level);
	std::cout << this->identifier << '\n';
	put_indent_level(indent_level + 1);
	std::cout << "be" << '\n';
	this->expression->print(indent_level + 1);
}

Ast::Assignment::~Assignment() {
	delete this->expression;
}

// Number
void Ast::Number::print(size_t indent_level) {
	put_indent_level(indent_level);
	std::cout << this->value << '\n';
}

Ast::Number::~Number() {}

// Identifier
void Ast::Identifier::print(size_t indent_level) {
	put_indent_level(indent_level);
	std::cout << this->value << '\n';
}

Ast::Identifier::~Identifier() {}
