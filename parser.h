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
    void markAsLandingInstruction() {_landing_instr = true;}
    bool isLandingInstruction() {return _landing_instr;}
  private:
    bool _landing_instr = false;
};
class LETInstruction : public Instruction {
  public:
    LETInstruction(VarIntValueToken *var,
                   IntValueToken *rhs,
                   OpToken *op = nullptr,
                   IntValueToken *lhs = nullptr);
  private:
    VarIntValueToken *_var;
    IntValueToken *_rhs;
    OpToken *_op;
    IntValueToken *_lhs;
};
class IFInstruction : public Instruction {
  public:
    IFInstruction(IntValueToken *lhs,
                  CmpToken *cmp,
                  IntValueToken *rhs,
                  ConstIntValueToken *label);
  private:
    IntValueToken *_lhs;
    CmpToken *_cmp;
    IntValueToken *_rhs;
    ConstIntValueToken *_label;
};
class PRINTInstruction : public Instruction {
  public:
    PRINTInstruction(StringValueToken *str);
  private:
    StringValueToken *_str;
};
class PRINTLNInstruction : public Instruction {
  public:
    PRINTLNInstruction(StringValueToken *str);
  private:
    StringValueToken *_str;
};


class BASICParser
{
  public:
    BASICParser() : _mod("BASIC", _global_ctx), _builder(_global_ctx) {}
    // ~BASICParser();

    bool parseFromTokenList(const std::vector<Token *> &tk_lst);
    llvm::Module *generateModule();
    void dumpMap();

  private:
    std::map<int, Instruction *> _instrs;
    std::set<int> _jump_landings;
    llvm::IRBuilder<> _builder;
    llvm::LLVMContext _global_ctx;
    llvm::Module _mod;

    bool _make_let(const std::vector<Token *> &tk_list, int &curr_pos, int label);
    bool _make_if(const std::vector<Token *> &tk_list, int &curr_pos, int label);
    bool _make_print(const std::vector<Token *> &tk_list, int &curr_pos, int label);
    bool _make_println(const std::vector<Token *> &tk_list, int &curr_pos, int label);
    
};


#endif  // PARSER_H_
