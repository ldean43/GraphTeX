#pragma once
#include "ast.hpp"
#include <unordered_map>

class Evaluator {
    private:
        float result_;
        float evaluateHelper(const Expr* expr);
    public:
        Expr* ast_;
        std::unordered_map<std::string, float> vars_;
        explicit Evaluator
        (Expr* expr, std::unordered_map<std::string, float> vars):
        ast_(expr), vars_(vars) {};
        ~Evaluator() {
            delete ast_;
        }
        float evaluate();
};