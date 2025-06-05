#include "evaluator.hpp"
#include "utils.hpp"
#include <cmath>
#include <iostream>

float Evaluator::evaluateHelper(const Expr* expr) {
    float eval;
    float zero = 1e-6;
    std::string func;
    switch (expr->type_) {
        case Type::NUM:
            return ((Num*)expr)->value_;
        case Type::VAR: {
            Var* var = (Var*)expr;
            if (vars_.count(var->value_)) {
                return vars_.at(var->value_);
            }
            throw std::runtime_error ("undefined variable");
        }
        case Type::OP: {
            Op* op = (Op*)expr;
            if (op->op_ == '+') {
                return evaluateHelper(op->e1_) + evaluateHelper(op->e2_);
            } else if (op->op_ == '-') {
                return evaluateHelper(op->e1_) - evaluateHelper(op->e2_);
            } else if (op->op_ == '*') {
                return evaluateHelper(op->e1_) * evaluateHelper(op->e2_);
            } else if (op->op_ == '/') {
                float eval = evaluateHelper(op->e2_);
                if (abs(eval) > zero) {
                    return evaluateHelper(op->e1_) / eval;
                }
                return NAN;
            } else if (op->op_ == '^') {
                return pow(evaluateHelper(op->e1_), evaluateHelper(op->e2_));
            }
            throw std::runtime_error ("evaluating error: invalid operator");
        }
        case Type::FRAC: {
            Frac* frac = (Frac*)expr;
            eval = evaluateHelper(frac->denominator_);
            if (abs(eval) > zero) {
                return evaluateHelper(frac->numerator_) / eval;
            }
            return NAN;
        }
        case Type::SQRT: {
            Sqrt* sqrt = (Sqrt*)expr;
            float root = evaluateHelper(sqrt->root_);
            if (abs(root) > zero) {
                return pow(evaluateHelper(sqrt->e_), 1.0/evaluateHelper(sqrt->root_));
            }
            return NAN;
        }
        case Type::LOG: {
            Log* log_expr = (Log*)expr;
            float base = evaluateHelper(log_expr->base_);
            eval = evaluateHelper(log_expr->e_);
            if (abs(base) > zero) {
                float denom = log(base);
                if (abs(denom) > zero && abs(eval) > zero) {
                    return log(eval) / denom;
                }
            }
            return NAN;
        }
        case Type::LN: {
            Ln* ln = (Ln*)expr;
            eval = evaluateHelper(ln->e_);
            if (abs(eval) > zero) {
                return log(eval);
            }
            return NAN;
        }
        case Type::LG: {
            Lg* lg = (Lg*)expr;
            eval = evaluateHelper(lg->e_);
            if (abs(eval) > zero) {
                return log2(evaluateHelper(lg->e_));
            }
            return NAN;
        }
        case Type::TRIG: {
            Trig* trig = (Trig*)expr;
            func = trig->func_;
            if (func == "sin") {
                return sin(evaluateHelper(trig->e_));
            } else if (func == "cos") {
                return cos(evaluateHelper(trig->e_));
            } else if (func == "tan") {
                float denom = cos(evaluateHelper(trig->e_));
                if (abs(denom) > zero) {
                    return sin(evaluateHelper(trig->e_))/denom;
                }
                return NAN;
            } else if (func == "csc") {
                eval = evaluateHelper(trig->e_);
                if (abs(eval) > zero) {
                    return 1.0/sin(evaluateHelper(trig->e_));
                }
                return NAN;
            } else if (func == "sec") {
                eval = evaluateHelper(trig->e_);
                if (abs(eval) > zero) {
                    return 1.0/cos(evaluateHelper(trig->e_));
                } else {
                    throw std::runtime_error ("evaluating error: division by 0");
                }
            } else if (func == "cot") {
                float denom = sin(evaluateHelper(trig->e_));
                if (abs(denom) > zero) {
                    return cos(evaluateHelper(trig->e_))/denom;
                }
                return NAN;
            } else if (func == "arcsin") {
                return asin(evaluateHelper(trig->e_));
            } else if (func == "arccos") {
                return acos(evaluateHelper(trig->e_));
            } else if (func == "arctan") {
                return atan(evaluateHelper(trig->e_));
            } else if (func == "arccsc") {
                eval = evaluateHelper(trig->e_);
                if (abs(eval) > zero) {
                     return asin(1.0/eval);
                }
                return NAN;
            } else if (func == "arcsec") {
                eval = evaluateHelper(trig->e_);
                if (abs(eval) > 0) {
                     return acos(1.0/eval);
                }
                return NAN;
            } else if (func == "arccot") {
                eval = evaluateHelper(trig->e_);
                if (abs(eval) > 0) {
                     return atan(1.0/eval);
                }
                return NAN;
            } else if (func == "sinh") {
                return sinh(evaluateHelper(trig->e_));
            } else if (func == "cosh") {
                return cosh(evaluateHelper(trig->e_));
            } else if (func == "tanh") {
                return tanh(evaluateHelper(trig->e_));
            }
        }
        case Type::ABS:
            return abs(evaluateHelper(((Abs*)expr)->e_));
        default:
            throw std::runtime_error ("evaluating error: invalid expression");
    }
}

Evaluator* Evaluator::copy() {
    return new Evaluator(ast_->copy(),{});
}

float Evaluator::evaluate() {
    result_ = evaluateHelper(ast_);
    return result_;
}