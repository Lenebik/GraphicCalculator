#include "Parser.h"

#include <utility>

namespace {

AstPtr makeNumber(double v) {
    auto n = std::make_unique<AstNode>();
    n->kind = NodeKind::Number;
    n->value = v;
    return n;
}

AstPtr makeBinary(NodeKind k, AstPtr l, AstPtr r) {
    auto n = std::make_unique<AstNode>();
    n->kind = k;
    n->left = std::move(l);
    n->right = std::move(r);
    return n;
}

}  // namespace

AstPtr Parser::parse(const std::vector<Token>& tokens) {
    m_error.clear();
    m_tokens = &tokens;
    m_pos = 0;

    if (tokens.empty() || tokens.front().type == TokenType::End) {
        m_error = "Пустое выражение";
        return nullptr;
    }

    AstPtr root = parseExpression();
    if (m_error.empty() && peek().type != TokenType::End) {
        m_error = "Лишний символ в выражении";
        return nullptr;
    }
    if (!m_error.empty()) {
        return nullptr;
    }
    return root;
}

bool Parser::match(TokenType t) {
    if (peek().type == t) {
        ++m_pos;
        return true;
    }
    return false;
}

AstPtr Parser::parseExpression() {
    AstPtr left = parseTerm();
    if (!m_error.empty()) return nullptr;
    while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
        TokenType op = consume().type;
        AstPtr right = parseTerm();
        if (!m_error.empty()) return nullptr;
        left = makeBinary(op == TokenType::Plus ? NodeKind::Add : NodeKind::Sub,
                          std::move(left), std::move(right));
    }
    return left;
}

AstPtr Parser::parseTerm() {
    AstPtr left = parseUnary();
    if (!m_error.empty()) return nullptr;
    while (peek().type == TokenType::Star || peek().type == TokenType::Slash) {
        TokenType op = consume().type;
        AstPtr right = parseUnary();
        if (!m_error.empty()) return nullptr;
        left = makeBinary(op == TokenType::Star ? NodeKind::Mul : NodeKind::Div,
                          std::move(left), std::move(right));
    }
    return left;
}

AstPtr Parser::parseUnary() {
    if (peek().type == TokenType::Plus) {
        consume();
        return parseUnary();
    }
    if (peek().type == TokenType::Minus) {
        consume();
        AstPtr operand = parseUnary();
        if (!m_error.empty()) return nullptr;
        auto n = std::make_unique<AstNode>();
        n->kind = NodeKind::UnaryMinus;
        n->left = std::move(operand);
        return n;
    }
    return parsePower();
}

AstPtr Parser::parsePower() {
    AstPtr base = parsePrimary();
    if (!m_error.empty()) return nullptr;
    if (peek().type == TokenType::Caret) {
        consume();
        // Right-associative: handle next unary so that -x^-y parses naturally.
        AstPtr exponent = parseUnary();
        if (!m_error.empty()) return nullptr;
        return makeBinary(NodeKind::Pow, std::move(base), std::move(exponent));
    }
    return base;
}

AstPtr Parser::parsePrimary() {
    const Token& tok = peek();
    switch (tok.type) {
        case TokenType::Number: {
            consume();
            return makeNumber(tok.number);
        }
        case TokenType::Variable: {
            consume();
            auto n = std::make_unique<AstNode>();
            n->kind = NodeKind::Variable;
            n->name = tok.text;
            return n;
        }
        case TokenType::Constant: {
            consume();
            auto n = std::make_unique<AstNode>();
            n->kind = NodeKind::Constant;
            n->name = tok.text;
            return n;
        }
        case TokenType::Function: {
            consume();
            if (peek().type != TokenType::LeftParen) {
                m_error = "Ожидалась '(' после " + tok.text;
                return nullptr;
            }
            consume();  // '('
            AstPtr arg = parseExpression();
            if (!m_error.empty()) return nullptr;
            if (peek().type != TokenType::RightParen) {
                m_error = "Ожидалась ')' после аргумента " + tok.text;
                return nullptr;
            }
            consume();  // ')'
            auto n = std::make_unique<AstNode>();
            n->kind = NodeKind::Function;
            n->name = tok.text;
            n->left = std::move(arg);
            return n;
        }
        case TokenType::LeftParen: {
            consume();
            AstPtr inside = parseExpression();
            if (!m_error.empty()) return nullptr;
            if (peek().type != TokenType::RightParen) {
                m_error = "Несбалансированные скобки";
                return nullptr;
            }
            consume();
            return inside;
        }
        case TokenType::End:
            m_error = "Неожиданный конец выражения";
            return nullptr;
        default:
            m_error = "Неожиданный символ: " + tok.text;
            return nullptr;
    }
}
