#ifndef LEXER_H_
#define LEXER_H_

#include <istream>
#include <iostream>
#include <sstream>
#include <vector>

#include "tokens.h"

class BASICLexer {
  public:
    ~BASICLexer() {
        for (auto it : _token_list) delete it;
    }

    bool readFromStream(std::istream &in_stream);
    std::vector<Token *> getTokens();

  private:
    std::vector<Token *> _token_list;

    bool _push_instruction(std::stringstream &rest);
    bool _push_LET(std::stringstream &rest);
    bool _push_IF(std::stringstream &rest);
    bool _push_PRINT(std::stringstream &rest);
    bool _push_PRINTLN(std::stringstream &rest);

    bool _push_op(std::stringstream &rest);
    bool _push_cmp(std::stringstream &rest);
    bool _push_int_or_var(std::stringstream &rest);
};

#endif  // LEXER_H_
