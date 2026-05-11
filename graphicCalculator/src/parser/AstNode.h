#pragma once

#include <memory>
#include <string>

enum class NodeKind {
    Number,
    Variable,
    Constant,
    UnaryMinus,
    Add,
    Sub,
    Mul,
    Div,
    Pow,
    Function
};

struct AstNode {
    NodeKind kind;
    double value = 0.0;
    std::string name;
    std::unique_ptr<AstNode> left;
    std::unique_ptr<AstNode> right;
};

using AstPtr = std::unique_ptr<AstNode>;
