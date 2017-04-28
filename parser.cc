#include <vector>
#include <iostream>  // Remove plz
#include "tokens.h"
#include "parser.h"

bool BASICParser::parseFromTokenList(const std::vector<Token *> &tk_lst) {
    int curr_pos = 0;
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

bool BASICParser::_make_let(const std::vector<Token *> &tk_lst, int &curr_pos, int label) {
    if (tk_lst[curr_pos + 3]->getName() == "EOLToken") {
        _instrs[label] = new LETInstruction(
            static_cast<VarIntValueToken *>(tk_lst[curr_pos + 1]),
            static_cast<IntValueToken *>(tk_lst[curr_pos + 2])); 
        curr_pos += 3;
    } else {
        _instrs[label] = new LETInstruction(
            static_cast<VarIntValueToken *>(tk_lst[curr_pos + 1]),
            static_cast<IntValueToken *>(tk_lst[curr_pos + 2]),
            static_cast<OpToken *>(tk_lst[curr_pos + 3]),
            static_cast<IntValueToken *>(tk_lst[curr_pos + 2]));
        curr_pos += 5;
    }
    return true;
}

bool BASICParser::_make_if(const std::vector<Token *> &tk_lst, int &curr_pos, int label) {
    ConstIntValueToken *landing_label = static_cast<ConstIntValueToken *>(tk_lst[curr_pos + 4]);
    _jump_landings.insert(landing_label->getVal());
    _instrs[label] = new IFInstruction(
        static_cast<IntValueToken *>(tk_lst[curr_pos + 1]),
        static_cast<CmpToken *>(tk_lst[curr_pos + 2]),
        static_cast<IntValueToken *>(tk_lst[curr_pos + 3]),
        landing_label);
    curr_pos += 5;
    return true;
}

bool BASICParser::_make_print(const std::vector<Token *> &tk_lst, int &curr_pos, int label) {
    _instrs[label] = new PRINTInstruction(
        static_cast<StringValueToken *>(tk_lst[curr_pos + 1]));
    curr_pos += 2;
    return true;
}

bool BASICParser::_make_println(const std::vector<Token *> &tk_lst, int &curr_pos, int label) {
    _instrs[label] = new PRINTLNInstruction(
        static_cast<StringValueToken *>(tk_lst[curr_pos + 1]));
    curr_pos += 2;
    return true;
}

void BASICParser::dumpMap() {
    std::cout << "Jump landing locations:" << std::endl;
    for (auto it : _jump_landings) std::cout << it << std::endl;
    std::cout << "Number of instructions in map: " << _instrs.size() << std::endl;
}

PRINTInstruction::PRINTInstruction(StringValueToken *str) : _str(str) {}
PRINTLNInstruction::PRINTLNInstruction(StringValueToken *str) : _str(str) {}
LETInstruction::LETInstruction(VarIntValueToken *var,
                               IntValueToken *rhs,
                               OpToken *op,
                               IntValueToken *lhs)
  : _var(var), _rhs(rhs), _op(op), _lhs(lhs) {}
IFInstruction::IFInstruction(IntValueToken *lhs,
                             CmpToken *cmp,
                             IntValueToken *rhs,
                             ConstIntValueToken *label)
  : _lhs(lhs), _cmp(cmp), _rhs(rhs), _label(label) {}

