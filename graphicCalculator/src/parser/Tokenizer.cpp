#include "Tokenizer.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace {

const std::unordered_set<std::string>& functionNames() {
    static const std::unordered_set<std::string> kNames = {
        "sin", "cos", "tan",
        "asin", "acos", "atan",
        "sinh", "cosh", "tanh",
        "log", "ln", "sqrt", "abs", "exp"
    };
    return kNames;
}

bool isOpeningSide(TokenType t) {
    return t == TokenType::Number
        || t == TokenType::Variable
        || t == TokenType::Constant
        || t == TokenType::RightParen;
}

bool isClosingSide(TokenType t) {
    return t == TokenType::Number
        || t == TokenType::Variable
        || t == TokenType::Constant
        || t == TokenType::Function
        || t == TokenType::LeftParen;
}

}  // namespace

bool Tokenizer::isFunctionName(const std::string& name) {
    return functionNames().contains(name);
}

bool Tokenizer::isConstantName(const std::string& name) {
    return name == "pi" || name == "e";
}

std::vector<Token> Tokenizer::tokenize(const std::string& input) {
    m_error.clear();
    std::vector<Token> tokens;
    tokens.reserve(input.size());

    const size_t n = input.size();
    size_t i = 0;
    while (i < n) {
        char c = input[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            ++i;
            continue;
        }

        const int pos = static_cast<int>(i);

        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            size_t j = i;
            bool seenDot = false;
            while (j < n && (std::isdigit(static_cast<unsigned char>(input[j])) || input[j] == '.')) {
                if (input[j] == '.') {
                    if (seenDot) {
                        m_error = "Некорректное число";
                        return {};
                    }
                    seenDot = true;
                }
                ++j;
            }
            const std::string numText = input.substr(i, j - i);
            try {
                Token t;
                t.type = TokenType::Number;
                t.text = numText;
                t.number = std::stod(numText);
                t.position = pos;
                tokens.push_back(std::move(t));
            } catch (...) {
                m_error = "Некорректное число";
                return {};
            }
            i = j;
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(c))) {
            size_t j = i;
            while (j < n && (std::isalpha(static_cast<unsigned char>(input[j]))
                          || std::isdigit(static_cast<unsigned char>(input[j])))) {
                ++j;
            }
            std::string ident = input.substr(i, j - i);
            // Variable x must be lowercase as per spec.
            if (ident == "x") {
                Token t{TokenType::Variable, ident, 0.0, pos};
                tokens.push_back(std::move(t));
            } else if (isConstantName(ident)) {
                Token t{TokenType::Constant, ident, 0.0, pos};
                tokens.push_back(std::move(t));
            } else if (isFunctionName(ident)) {
                Token t{TokenType::Function, ident, 0.0, pos};
                tokens.push_back(std::move(t));
            } else {
                m_error = "Неизвестный идентификатор: " + ident;
                return {};
            }
            i = j;
            continue;
        }

        Token t;
        t.position = pos;
        t.text = std::string(1, c);
        switch (c) {
            case '+': t.type = TokenType::Plus; break;
            case '-': t.type = TokenType::Minus; break;
            case '*': t.type = TokenType::Star; break;
            case '/': t.type = TokenType::Slash; break;
            case '^': t.type = TokenType::Caret; break;
            case '(': t.type = TokenType::LeftParen; break;
            case ')': t.type = TokenType::RightParen; break;
            default:
                m_error = "Недопустимый символ: ";
                m_error += c;
                return {};
        }
        tokens.push_back(std::move(t));
        ++i;
    }

    // Insert implicit multiplication where adjacent tokens form an opening/closing pair.
    std::vector<Token> withImplicit;
    withImplicit.reserve(tokens.size());
    for (size_t k = 0; k < tokens.size(); ++k) {
        if (!withImplicit.empty()
            && isOpeningSide(withImplicit.back().type)
            && isClosingSide(tokens[k].type)) {
            Token mul;
            mul.type = TokenType::Star;
            mul.text = "*";
            mul.position = tokens[k].position;
            withImplicit.push_back(std::move(mul));
        }
        withImplicit.push_back(tokens[k]);
    }

    Token end;
    end.type = TokenType::End;
    end.position = static_cast<int>(n);
    withImplicit.push_back(std::move(end));

    return withImplicit;
}
