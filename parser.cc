#include <vector>
#include <iostream>  // Remove plz
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include "tokens.h"
#include "parser.h"

BASICParser::BASICParser() {
    _mod.reset(new llvm::Module("BASIC", _global_ctx));
    _builder.reset(new llvm::IRBuilder<>(_global_ctx));
}

bool BASICParser::parseFromTokenList(const std::vector<Token *> &tk_lst) {
    unsigned int curr_pos = 0;
    while (curr_pos < tk_lst.size()) {
        int label = static_cast<ConstIntValueToken *>(tk_lst[curr_pos])->getVal();
        ++curr_pos;
        std::string next_token = tk_lst[curr_pos]->getName();
        if (next_token == "LETToken") {
            if (!_make_let(tk_lst, curr_pos, label)) return false;
        } else if (next_token == "IFToken") {
            if (!_make_if(tk_lst, curr_pos, label)) return false;
        } else if (next_token == "PRINTToken") {
            if (!_make_print(tk_lst, curr_pos, label)) return false;
        } else if (next_token == "PRINTLNToken") {
            if (!_make_println(tk_lst, curr_pos, label)) return false;
        } else {
            std::cout << "Invalid token '" << next_token << "' expecting instruction\n";
            return false;
        }
        if (tk_lst[curr_pos]->getName() != "EOLToken") {
            std::cout << "Trailing tokens at end of line (label: " << label << ")\n";
            return false;
        }
        ++curr_pos;
    }
    return true;
}

llvm::Module *BASICParser::generateModule() {
    if (!_create_functions()) return nullptr;
    if (!_create_blocks()) return nullptr;
    if (!_create_vars()) return nullptr;

    for (auto it : _instrs) {
        if (_blocks.find(it.first) != _blocks.end()) {
            _builder->CreateBr(_blocks[it.first]);
            _builder->SetInsertPoint(_blocks[it.first]);
        }
        if (!it.second->addToBuilder(_builder.get(), _mod.get())) return nullptr;
    }

    _builder->CreateBr(_blocks.rbegin()->second);
    _builder->SetInsertPoint(_blocks.rbegin()->second);
    _builder->CreateRet(
        llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(_global_ctx),
            0));
    return _mod.get();
}

bool BASICParser::_create_functions() {
    // printf
    std::vector<llvm::Type *> printf_args = {llvm::Type::getInt8PtrTy(_global_ctx)};
    llvm::FunctionType *printf_type = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(_global_ctx),
        printf_args,
        true);
    _printf = llvm::Function::Create(
        printf_type,
        llvm::Function::ExternalLinkage,
        "printf",
        _mod.get());
    _printf->setCallingConv(llvm::CallingConv::C);
    _printf->addAttribute(1, llvm::Attribute::NoCapture);
    // main
    llvm::FunctionType *main_type = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(_global_ctx),
        std::vector<llvm::Type *>(),
        false);
    _main = llvm::Function::Create(
        main_type,
        llvm::Function::ExternalLinkage,
        "main",
        _mod.get());
    _main->setCallingConv(llvm::CallingConv::C);
    return true;
}

bool BASICParser::_create_blocks() {
    // Get all the labels that the blocks should be attached to
    std::set<int> bb_labels;
    bb_labels.insert(0);
    for (auto label : _jump_landings) bb_labels.insert(label);
    for (auto fallthrough_it : _jump_fallthrough) {
        auto instr_it = _instrs.find(fallthrough_it);
        ++instr_it;
        if (instr_it != _instrs.end()) {
            bb_labels.insert(instr_it->first);
        }
    }
    bb_labels.insert(_instrs.rbegin()->first + 1);
    std::cout << "Split into " << bb_labels.size() << " blocks\n";
    // Generate a block for each label
    for (auto label : bb_labels) {
        _blocks[label] = llvm::BasicBlock::Create(
            _global_ctx, std::to_string(label), _main, 0);
    }
    _builder->SetInsertPoint(_blocks[0]);
    return true;
}

bool BASICParser::_create_vars() {
    llvm::ArrayType *arr_type = llvm::ArrayType::get(
        llvm::Type::getInt32Ty(_global_ctx),
        26);
    llvm::ConstantAggregateZero *arr_default = llvm::ConstantAggregateZero::get(arr_type);
    llvm::GlobalVariable *g_vars = new llvm::GlobalVariable(
        *_mod,
        arr_type,
        false,
        llvm::GlobalValue::ExternalLinkage,
        0,
        "vars");
    g_vars->setInitializer(arr_default);
    return true;
}

bool BASICParser::_make_let(const std::vector<Token *> &tk_lst, unsigned int &curr_pos, int label) {
    if (tk_lst[curr_pos + 3]->getName() == "EOLToken") {
        _instrs[label] = new LETInstruction(
            label,
            static_cast<VarIntValueToken *>(tk_lst[curr_pos + 1]),
            static_cast<IntValueToken *>(tk_lst[curr_pos + 2])); 
        curr_pos += 3;
    } else {
        _instrs[label] = new LETInstruction(
            label,
            static_cast<VarIntValueToken *>(tk_lst[curr_pos + 1]),
            static_cast<IntValueToken *>(tk_lst[curr_pos + 2]),
            static_cast<OpToken *>(tk_lst[curr_pos + 3]),
            static_cast<IntValueToken *>(tk_lst[curr_pos + 2]));
        curr_pos += 5;
    }
    return true;
}

bool BASICParser::_make_if(const std::vector<Token *> &tk_lst, unsigned int &curr_pos, int label) {
    ConstIntValueToken *landing_label = static_cast<ConstIntValueToken *>(tk_lst[curr_pos + 4]);
    _jump_landings.insert(landing_label->getVal());
    _jump_fallthrough.insert(label);
    _instrs[label] = new IFInstruction(
        label,
        static_cast<IntValueToken *>(tk_lst[curr_pos + 1]),
        static_cast<CmpToken *>(tk_lst[curr_pos + 2]),
        static_cast<IntValueToken *>(tk_lst[curr_pos + 3]),
        landing_label);
    curr_pos += 5;
    return true;
}

bool BASICParser::_make_print(const std::vector<Token *> &tk_lst, unsigned int &curr_pos, int label) {
    if (tk_lst[curr_pos + 1]->getName() == "StringValueToken") {
        _instrs[label] = new PRINTInstruction(
            label,
            static_cast<StringValueToken *>(tk_lst[curr_pos + 1]));
    } else {
        _instrs[label] = new PRINTInstruction(
            label,
            static_cast<VarIntValueToken *>(tk_lst[curr_pos + 1]));
    }
    curr_pos += 2;
    return true;
}

bool BASICParser::_make_println(const std::vector<Token *> &tk_lst, unsigned int &curr_pos, int label) {
    if (tk_lst[curr_pos + 1]->getName() == "StringValueToken") {
        _instrs[label] = new PRINTLNInstruction(
            label,
            static_cast<StringValueToken *>(tk_lst[curr_pos + 1]));
    } else {
        _instrs[label] = new PRINTLNInstruction(
            label,
            static_cast<VarIntValueToken *>(tk_lst[curr_pos + 1]));
    }
    curr_pos += 2;
    return true;
}

void BASICParser::dumpMap() {
    std::cout << "Jump landing locations:" << std::endl;
    for (auto it : _jump_landings) std::cout << it << std::endl;
    std::cout << "Number of instructions in map: " << _instrs.size() << std::endl;
}

// Instruction definitions
Instruction::Instruction(int lbl) : label(lbl) {}
llvm::Value *Instruction::_get_var(llvm::IRBuilder<> *builder, llvm::Module *mod, char var) {
    llvm::Value *index = llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(mod->getContext()), var - 65);
    auto vars = mod->getGlobalVariable("vars");
    llvm::Value *elm_ptr = builder->CreateGEP(vars, index);
    llvm::Value *elm = builder->CreateLoad(elm_ptr);
    return elm;
}

PRINTInstruction::PRINTInstruction(int label, StringValueToken *str)
  : Instruction(label), _str(str) {}
PRINTInstruction::PRINTInstruction(int label, VarIntValueToken *var)
  : Instruction(label), _var(var) {}
bool PRINTInstruction::addToBuilder(llvm::IRBuilder<> *builder, llvm::Module *mod) {
    std::cout << "Adding print\n";
    if (_str != nullptr) {
        auto str_ptr = builder->CreateGlobalStringPtr(_str->getVal());
        builder->CreateCall(
            mod->getFunction("printf"),
            std::vector<llvm::Value *>{str_ptr});
    } else {
        auto str_ptr = builder->CreateGlobalStringPtr("%d");
        llvm::Value *var = _get_var(builder, mod, _var->getVal());
        builder->CreateCall(
            mod->getFunction("printf"),
            std::vector<llvm::Value *>{str_ptr, var});
    }
    return true;
}

PRINTLNInstruction::PRINTLNInstruction(int label, StringValueToken *str)
  : Instruction(label), _str(str) {}
PRINTLNInstruction::PRINTLNInstruction(int label, VarIntValueToken *var)
  : Instruction(label), _var(var) {}
bool PRINTLNInstruction::addToBuilder(llvm::IRBuilder<> *builder, llvm::Module *mod) {
    std::cout << "Adding printLN\n";
    if (_str != nullptr) {
        auto str_ptr = builder->CreateGlobalStringPtr(_str->getVal() + '\n');
        builder->CreateCall(
            mod->getFunction("printf"),
            std::vector<llvm::Value *>{str_ptr});
    } else {
        auto str_ptr = builder->CreateGlobalStringPtr("%d\n");
        llvm::Value *var = _get_var(builder, mod, _var->getVal());
        builder->CreateCall(
            mod->getFunction("printf"),
            std::vector<llvm::Value *>{str_ptr, var});
    }
    return true;
}

LETInstruction::LETInstruction(int label,
                               VarIntValueToken *var,
                               IntValueToken *rhs,
                               OpToken *op,
                               IntValueToken *lhs)
  : Instruction(label), _var(var), _rhs(rhs), _op(op), _lhs(lhs) {}
bool LETInstruction::addToBuilder(llvm::IRBuilder<> *builder, llvm::Module *mod) {
    std::cout << "Adding Let\n";
    return true;
}

IFInstruction::IFInstruction(int label,
                             IntValueToken *lhs,
                             CmpToken *cmp,
                             IntValueToken *rhs,
                             ConstIntValueToken *true_label)
  : Instruction(label), _lhs(lhs), _cmp(cmp), _rhs(rhs), _true_label(true_label) {}
bool IFInstruction::addToBuilder(llvm::IRBuilder<> *builder, llvm::Module *mod) {
    std::cout << "Adding IF\n";
    return true;
}
