#pragma once

#include "AstNode.h"

class Evaluator {
public:
    // Evaluates the AST at the given x. Returns NaN on any domain or runtime error
    // (the caller treats NaN as "skip this point").
    static double evaluate(const AstNode* node, double x);
};
