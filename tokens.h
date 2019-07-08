
#ifndef TOKENS_H_
#define TOKENS_H_

#include <string>

// 
// Base token class
//
class Token {
  public:
    virtual std::string getName() const = 0;
};

//
// Tokens for syntax
//
class SyntaxToken : public Token {};
class EOLToken : public SyntaxToken {
    virtual std::string getName() const {return "EOLToken";}
};

//
// Tokens for values
//
class ValueToken : public Token {};
class StringValueToken : public ValueToken {
  public:
    StringValueToken(std::string val) {_val = val;}
    std::string getVal() {return _val;}
    virtual std::string getName() const {return "StringValueToken";}
  private:
    std::string _val;
};
class IntValueToken : public ValueToken {};
class ConstIntValueToken : public IntValueToken {
  public:
    ConstIntValueToken(int val) {_val = val;}
    int getVal() {return _val;}
    virtual std::string getName() const {return "ConstIntValueToken";}
  private:
    int _val;
};
class VarIntValueToken : public IntValueToken {
  public:
    VarIntValueToken(char val) {_val = val;}
    char getVal() {return _val;}
    virtual std::string getName() const {return "VarIntValueToken";}
  public:
    char _val;
};

//
// Tokens for operations
//
class OpToken : public Token {};
class PlusToken : public OpToken {
    virtual std::string getName() const {return "PlusToken";}
};
class MinusToken : public OpToken {
    virtual std::string getName() const {return "MinusToken";}
};
class MulToken : public OpToken {
    virtual std::string getName() const {return "MulToken";}
};
class DivToken : public OpToken {
    virtual std::string getName() const {return "DivToken";}
};

//
// Tokens for comparisons
//
class CmpToken : public Token {};
class EqToken : public CmpToken {
    virtual std::string getName() const {return "EqToken";}
};
class LtToken : public CmpToken {
    virtual std::string getName() const {return "LtToken";}
};
class GtToken : public CmpToken {
    virtual std::string getName() const {return "GtToken";}
};
class NeToken : public CmpToken {
    virtual std::string getName() const {return "NeToken";}
};
class LteToken : public CmpToken {
    virtual std::string getName() const {return "LteToken";}
};
class GteToken : public CmpToken {
    virtual std::string getName() const {return "GteToken";}
};

//
// Tokens for instructions
//
class InstrToken : public Token {};
class LETToken : public InstrToken {
    virtual std::string getName() const {return "LETToken";}
};
class IFToken : public InstrToken {
    virtual std::string getName() const {return "IFToken";}
};
class PRINTToken : public InstrToken {
    virtual std::string getName() const {return "PRINTToken";}
};
class PRINTLNToken : public InstrToken {
    virtual std::string getName() const {return "PRINTLNToken";}
};

#endif  // TOKENS_H_
