#include "Evaluator.h"

#include <cmath>
#include <limits>

namespace {

constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

double applyFunction(const std::string& name, double a) {
    if (name == "sin")  return std::sin(a);
    if (name == "cos")  return std::cos(a);
    if (name == "tan")  return std::tan(a);
    if (name == "asin") return (a < -1.0 || a > 1.0) ? kNaN : std::asin(a);
    if (name == "acos") return (a < -1.0 || a > 1.0) ? kNaN : std::acos(a);
    if (name == "atan") return std::atan(a);
    if (name == "sinh") return std::sinh(a);
    if (name == "cosh") return std::cosh(a);
    if (name == "tanh") return std::tanh(a);
    if (name == "log")  return (a <= 0.0) ? kNaN : std::log10(a);
    if (name == "ln")   return (a <= 0.0) ? kNaN : std::log(a);
    if (name == "sqrt") return (a < 0.0)  ? kNaN : std::sqrt(a);
    if (name == "abs")  return std::fabs(a);
    if (name == "exp")  return std::exp(a);
    return kNaN;
}

}  // namespace

double Evaluator::evaluate(const AstNode* node, double x) {
    if (!node) return kNaN;

    switch (node->kind) {
        case NodeKind::Number:
            return node->value;
        case NodeKind::Variable:
            return x;
        case NodeKind::Constant:
            if (node->name == "pi") return M_PI;
            if (node->name == "e")  return M_E;
            return kNaN;
        case NodeKind::UnaryMinus: {
            const double a = evaluate(node->left.get(), x);
            return -a;
        }
        case NodeKind::Add: {
            const double a = evaluate(node->left.get(), x);
            const double b = evaluate(node->right.get(), x);
            return a + b;
        }
        case NodeKind::Sub: {
            const double a = evaluate(node->left.get(), x);
            const double b = evaluate(node->right.get(), x);
            return a - b;
        }
        case NodeKind::Mul: {
            const double a = evaluate(node->left.get(), x);
            const double b = evaluate(node->right.get(), x);
            return a * b;
        }
        case NodeKind::Div: {
            const double a = evaluate(node->left.get(), x);
            const double b = evaluate(node->right.get(), x);
            if (b == 0.0) return kNaN;
            return a / b;
        }
        case NodeKind::Pow: {
            const double a = evaluate(node->left.get(), x);
            const double b = evaluate(node->right.get(), x);
            // Avoid producing complex results for negative base with non-integer exponent.
            if (a < 0.0 && std::floor(b) != b) return kNaN;
            const double r = std::pow(a, b);
            if (!std::isfinite(r)) return kNaN;
            return r;
        }
        case NodeKind::Function: {
            const double a = evaluate(node->left.get(), x);
            if (std::isnan(a)) return kNaN;
            return applyFunction(node->name, a);
        }
    }
    return kNaN;
}
