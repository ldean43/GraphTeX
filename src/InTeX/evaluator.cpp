#include "evaluator.hpp"
#include "utils.hpp"
#include <cmath>
#include <iostream>

float Evaluator::evaluateHelper(const Expr* expr) {
    float eval;
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
                if (eval != 0) {
                    return evaluateHelper(op->e1_) / eval;
                }
                throw std::runtime_error ("evaluating error: division by 0");
            } else if (op->op_ == '^') {
                return pow(evaluateHelper(op->e1_), evaluateHelper(op->e2_));
            }
            throw std::runtime_error ("evaluating error: invalid operator");
        }
        case Type::FRAC: {
            Frac* frac = (Frac*)expr;
            eval = evaluateHelper(frac->denominator_);
            if (eval != 0) {
                return evaluateHelper(frac->numerator_) / eval;
            }
            throw std::runtime_error ("evaluating error: division by 0");
        }
        case Type::SQRT: {
            Sqrt* sqrt = (Sqrt*)expr;
            return pow(evaluateHelper(sqrt->e_), 1.0/evaluateHelper(sqrt->root_));
        }
        case Type::LOG: {
            Log* log_expr = (Log*)expr;
            if (log_expr->base_ == 0) {
                throw std::runtime_error ("evaluating error: log base 0");
            }
            return log(evaluateHelper(log_expr->e_)) / log(evaluateHelper(log_expr->base_));
        }
        case Type::LN: {
            Ln* ln = (Ln*)expr;
            return log(evaluateHelper(ln->e_));
        }
        case Type::LG: {
            Lg* lg = (Lg*)expr;
            return log2(evaluateHelper(lg->e_));
        }
        case Type::TRIG: {
            Trig* trig = (Trig*)expr;
            func = trig->func_;
            if (func == "sin") {
                return sin(evaluateHelper(trig->e_));
            } else if (func == "cos") {
                return cos(evaluateHelper(trig->e_));
            } else if (func == "tan") {
                return tan(evaluateHelper(trig->e_));
            } else if (func == "csc") {
                eval = evaluateHelper(trig->e_);
                if (eval != 0) {
                    return 1.0/sin(evaluateHelper(trig->e_));
                } else {
                    throw std::runtime_error ("evaluating error: division by 0");
                }
            } else if (func == "sec") {
                eval = evaluateHelper(trig->e_);
                if (eval != 0) {
                    return 1.0/cos(evaluateHelper(trig->e_));
                } else {
                    throw std::runtime_error ("evaluating error: division by 0");
                }
            } else if (func == "cot") {
                eval = evaluateHelper(trig->e_);
                if (eval != 0) {
                    return 1.0/tan(evaluateHelper(trig->e_));
                } else {
                    throw std::runtime_error ("evaluating error: division by 0");
                }
            } else if (func == "arcsin") {
                return asin(evaluateHelper(trig->e_));
            } else if (func == "arccos") {
                return acos(evaluateHelper(trig->e_));
            } else if (func == "arctan") {
                return atan(evaluateHelper(trig->e_));
            } else if (func == "arccsc") {
                eval = evaluateHelper(trig->e_);
                if (eval != 0) {
                     return asin(1.0/evaluateHelper(trig->e_));
                } else {
                    throw std::runtime_error ("evaluating error: division by 0");
                }
            } else if (func == "arcsec") {
                eval = evaluateHelper(trig->e_);
                if (eval != 0) {
                     return acos(1.0/evaluateHelper(trig->e_));
                } else {
                    throw std::runtime_error ("evaluating error: division by 0");
                }
            } else if (func == "arccot") {
                eval = evaluateHelper(trig->e_);
                if (eval != 0) {
                     return atan(1.0/evaluateHelper(trig->e_));
                } else {
                    throw std::runtime_error ("evaluating error: division by 0");
                }
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

float Evaluator::evaluate() {
    result_ = evaluateHelper(ast_);
    return result_;
}