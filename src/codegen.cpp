#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <iostream>

#include "codegen.hpp"

struct Codegen {
	llvm::LLVMContext* context;
	llvm::Module* module;
	llvm::IRBuilder<>* builder;

	Codegen() {
		this->context = new llvm::LLVMContext();
		this->module = new llvm::Module("My cool jit", *(this->context));
		this->builder = new llvm::IRBuilder(*(this->context));
	}

	void codegen(Ast::Program* node);
	llvm::Value* codegen(Ast::Number* node);
};

void generate_executable(Ast::Program &program, std::string executable_name) {
	Codegen llvm_ir;
	llvm_ir.codegen(&program);

	// Generate object file
	auto TargetTriple = llvm::sys::getDefaultTargetTriple();
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	std::string Error;
	auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

	// Print an error and exit if we couldn't find the requested target.
	// This generally occurs if we've forgotten to initialise the
	// TargetRegistry or we have a bogus target triple.
	if (!Target) {
		llvm::errs() << Error;
	}

	auto CPU = "generic";
	auto Features = "";

	llvm::TargetOptions opt;
	auto RM = llvm::Optional<llvm::Reloc::Model>();
	auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

	llvm_ir.module->setDataLayout(TargetMachine->createDataLayout());
	llvm_ir.module->setTargetTriple(TargetTriple);

	std::string object_file_name = executable_name + ".o";

	std::error_code EC;
	llvm::raw_fd_ostream dest(object_file_name, EC, llvm::sys::fs::OF_None);

	if (EC) {
		llvm::errs() << "Could not open file: " << EC.message();
	}

	llvm::legacy::PassManager pass;
	auto FileType = llvm::CGFT_ObjectFile;

	if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
		llvm::errs() << "TargetMachine can't emit a file of this type";
	}

	pass.run(*(llvm_ir.module));
	dest.flush();

	std::string command = "clang-12 -o ";
	command += executable_name;
	command += " ";
	command += object_file_name;
	command += " && rm ";
	command += object_file_name;
	system(command.c_str());
}

void Codegen::codegen(Ast::Program* node) {
	// Declare printf
	std::vector<llvm::Type*> args;
	args.push_back(llvm::Type::getInt8PtrTy(*(this->context)));
	llvm::FunctionType *printfType = llvm::FunctionType::get(this->builder->getInt32Ty(), args, true); // `true` specifies the function as variadic
	llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", this->module);

	// Crate main function
	llvm::FunctionType* mainType = llvm::FunctionType::get(this->builder->getInt32Ty(), false);
	llvm::Function* main = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", this->module);
	llvm::BasicBlock* entry = llvm::BasicBlock::Create(*(this->context), "entry", main);
	this->builder->SetInsertPoint(entry);

	for (size_t i = 0; i < node->expressions.size(); i++) {
		if (dynamic_cast<Ast::Number*>(node->expressions[i])) {
			this->codegen(dynamic_cast<Ast::Number*>(node->expressions[i]));
		}
	}

	// Create return statement
	this->builder->CreateRet(llvm::ConstantInt::get(*(this->context), llvm::APInt(32, 0)));
}

llvm::Value* Codegen::codegen(Ast::Number* node) {
	llvm::Value* value = llvm::ConstantFP::get(*(this->context), llvm::APFloat(node->value));
	llvm::Function* print_function = this->module->getFunction("printf");

	if (!print_function) {
		std::cout << "No print funciont :(" << '\n';
	}

	// Format string
	std::vector<llvm::Value*> printArgs;
	llvm::Value* format_str = this->builder->CreateGlobalStringPtr("%g\n");
	printArgs.push_back(format_str);
	printArgs.push_back(value);
	this->builder->CreateCall(this->module->getFunction("printf"), printArgs);
	return value;
}
