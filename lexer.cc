#include <istream>
#include <sstream>
#include <string>
#include <stdio.h>

#include "lexer.h"
#include "tokens.h"

bool BASICLexer::readFromStream(std::istream &in_stream) {
    std::string line;
    while (std::getline(in_stream, line)) {
        std::stringstream ss(line);
        if (!_push_int_or_var(ss)) return false;
        if (!_push_instruction(ss)) return false;
        _token_list.push_back(new EOLToken());
    }
    return true;
}

std::vector<Token *> BASICLexer::getTokens() {
    return _token_list;
}

bool BASICLexer::_push_instruction(std::stringstream &ss) {
    std::string instr;
    ss >> instr;
    if (instr == "LET") {
        _token_list.push_back(new LETToken());
        _push_LET(ss);
    } else if (instr == "IF") {
        _token_list.push_back(new IFToken());
        _push_IF(ss);
    } else if (instr == "PRINT") {
        _token_list.push_back(new PRINTToken());
        _push_const_str(ss);
    } else if (instr == "PRINTLN") {
        _token_list.push_back(new PRINTLNToken());
        _push_const_str(ss);
    } else {
        printf("Unknown instruction %s\n", instr.c_str());
        return false;
    }
    return true;
}

bool BASICLexer::_push_LET(std::stringstream &ss) {
    std::string unused;
    char lhs;
    ss >> lhs >> unused;
    _token_list.push_back(new VarIntValueToken(lhs));
    if (unused != "=") {
        printf("LET instr must be in the form 'LET X = <expression>'\n");
        return false;
    }
    if (!_push_int_or_var(ss)) return false;
    if (ss.peek() == EOF) return true;
    if (!_push_op(ss)) return false;
    if (!_push_int_or_var(ss)) return false;
    return true;
}

bool BASICLexer::_push_IF(std::stringstream &ss) {
    if (!_push_int_or_var(ss)) return false;
    if (!_push_cmp(ss)) return false;
    if (!_push_int_or_var(ss)) return false;
    std::string then, gto;
    ss >> then >> gto;
    if (then != "THEN" || gto != "GOTO") {
        printf("IF must follow format of IF <cond> THEN GOTO L\n");
        return false;
    }
    if (!_push_int_or_var(ss)) return false;
    return true;
}

bool BASICLexer::_push_int_or_var(std::stringstream &ss) {
    int i_rhs1;
    ss >> i_rhs1;
    if (!ss.fail()) {
        _token_list.push_back(new ConstIntValueToken(i_rhs1));
    } else {
        ss.clear();
        char c_rhs1;
        ss >> c_rhs1;
        _token_list.push_back(new VarIntValueToken(c_rhs1));
    }
    return true;
}

bool BASICLexer::_push_op(std::stringstream &ss) {
    char op;
    ss >> op;
    if (op == '+') {
        _token_list.push_back(new PlusToken());
    } else if (op == '-') {
        _token_list.push_back(new MinusToken());
    } else if (op == '*') {
        _token_list.push_back(new MulToken());
    } else if (op == '/') {
        _token_list.push_back(new DivToken());
    } else {
        printf("Unknown op: %c\n", op);
        return false;
    }
    return true;
}

bool BASICLexer::_push_cmp(std::stringstream &ss) {
    std::string op;
    ss >> op;
    if (op == "=") {
        _token_list.push_back(new EqToken());
    } else if (op == ">") {
        _token_list.push_back(new GtToken());
    } else if (op == "<") {
        _token_list.push_back(new LtToken());
    } else if (op == "<>") {
        _token_list.push_back(new NeToken());
    } else if (op == "<=") {
        _token_list.push_back(new LteToken());
    } else if (op == ">=") {
        _token_list.push_back(new GteToken());
    } else {
        printf("Unknown operation: %s\n", op.c_str());
        return false;
    }
    return true;
}

bool BASICLexer::_push_const_str(std::stringstream &ss) {
    std::string temp;
    ss >> temp;
    if (temp[0] != '"') {
        _token_list.push_back(new VarIntValueToken(temp[0]));
    } else {
        std::string const_str;
        std::getline(ss, const_str);
        const_str = temp + const_str;
        _token_list.push_back(
            new StringValueToken(const_str.substr(1, const_str.length() - 2)));
    }
    return true;
}
