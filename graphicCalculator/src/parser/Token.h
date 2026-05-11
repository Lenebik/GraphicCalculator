#pragma once

#include <string>

enum class TokenType {
    Number,
    Variable,
    Constant,
    Function,
    Plus,
    Minus,
    Star,
    Slash,
    Caret,
    LeftParen,
    RightParen,
    End
};

struct Token {
    TokenType type;
    std::string text;
    double number = 0.0;
    int position = 0;
};
