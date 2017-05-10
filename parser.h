#ifndef PARSER_H_
#define PARSER_H_

#include <vector>
#include <map>
#include <set>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include "tokens.h"

class Instruction {
  public:
    int label;
    Instruction(int lbl);
    virtual bool addToBuilder(llvm::IRBuilder<> *builder, llvm::Module *mod) = 0;
  protected:
    llvm::Value *_get_var_ptr(llvm::IRBuilder<> *builder, llvm::Module *mod, char var);
    llvm::Value *_get_var(llvm::IRBuilder<> *builder, llvm::Module *mod, char var);
    llvm::Value *_set_var(llvm::IRBuilder<> *builder, llvm::Module *mod, char var, llvm::Value *val);
};
class LETInstruction : public Instruction {
  public:
    LETInstruction(int label,
                   VarIntValueToken *var,
                   IntValueToken *rhs,
                   OpToken *op = nullptr,
                   IntValueToken *lhs = nullptr);
    virtual bool addToBuilder(llvm::IRBuilder<> *builder, llvm::Module *mod) override;
  private:
    VarIntValueToken *_var;
    IntValueToken *_rhs;
    OpToken *_op;
    IntValueToken *_lhs;
};
class IFInstruction : public Instruction {
  public:
    IFInstruction(int label,
                  IntValueToken *lhs,
                  CmpToken *cmp,
                  IntValueToken *rhs,
                  ConstIntValueToken *true_label);
    virtual bool addToBuilder(llvm::IRBuilder<> *builder, llvm::Module *mod) override;
  private:
    IntValueToken *_lhs;
    CmpToken *_cmp;
    IntValueToken *_rhs;
    ConstIntValueToken *_true_label;
};
class PRINTInstruction : public Instruction {
  public:
    PRINTInstruction(int label, StringValueToken *str);
    PRINTInstruction(int label, VarIntValueToken *var);
    virtual bool addToBuilder(llvm::IRBuilder<> *builder, llvm::Module *mod) override;
  private:
    StringValueToken *_str = nullptr;
    VarIntValueToken *_var = nullptr;
};
class PRINTLNInstruction : public Instruction {
  public:
    PRINTLNInstruction(int label, StringValueToken *str);
    PRINTLNInstruction(int label, VarIntValueToken *var);
    virtual bool addToBuilder(llvm::IRBuilder<> *builder, llvm::Module *mod) override;
  private:
    StringValueToken *_str = nullptr;
    VarIntValueToken *_var = nullptr;
};


class BASICParser
{
  public:
    BASICParser();
    // ~BASICParser();

    bool parseFromTokenList(const std::vector<Token *> &tk_lst);
    llvm::Module *generateModule();
    void dumpMap();

  private:
    std::map<int, llvm::BasicBlock *> _blocks;
    std::map<int, Instruction *> _instrs;
    std::set<int> _jump_landings;
    std::set<int> _jump_fallthrough;
    llvm::LLVMContext _global_ctx;
    std::unique_ptr<llvm::Module> _mod;
    std::unique_ptr<llvm::IRBuilder<>> _builder;

    llvm::Function *_main;
    llvm::Function *_printf;

    bool _make_let(const std::vector<Token *> &tk_list, unsigned int &curr_pos, int label);
    bool _make_if(const std::vector<Token *> &tk_list, unsigned int &curr_pos, int label);
    bool _make_print(const std::vector<Token *> &tk_list, unsigned int &curr_pos, int label);
    bool _make_println(const std::vector<Token *> &tk_list, unsigned int &curr_pos, int label);
    
    bool _create_functions();
    bool _create_blocks();
    bool _create_vars();
};


#endif  // PARSER_H_
