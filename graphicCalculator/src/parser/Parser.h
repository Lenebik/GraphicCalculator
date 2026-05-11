#pragma once

#include <string>
#include <vector>

#include "AstNode.h"
#include "Token.h"

class Parser {
public:
    AstPtr parse(const std::vector<Token>& tokens);

    bool hasError() const { return !m_error.empty(); }
    const std::string& error() const { return m_error; }

private:
    AstPtr parseExpression();
    AstPtr parseTerm();
    AstPtr parseUnary();
    AstPtr parsePower();
    AstPtr parsePrimary();

    const Token& peek() const { return m_tokens->at(m_pos); }
    const Token& consume() { return m_tokens->at(m_pos++); }
    bool match(TokenType t);

    const std::vector<Token>* m_tokens = nullptr;
    size_t m_pos = 0;
    std::string m_error;
};
