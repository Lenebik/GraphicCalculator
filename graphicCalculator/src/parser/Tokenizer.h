#pragma once

#include <string>
#include <vector>

#include "Token.h"

class Tokenizer {
public:
    std::vector<Token> tokenize(const std::string& input);

    bool hasError() const { return !m_error.empty(); }
    const std::string& error() const { return m_error; }

private:
    static bool isFunctionName(const std::string& name);
    static bool isConstantName(const std::string& name);

    std::string m_error;
};
