#pragma once
#include "ast.hpp"
#include <string>

bool is_float(std::string str);

bool is_var(std::string str);

void print_indented(const unsigned int depth, const std::string& str);

void print_ast(const Expr* expr, const std::string prefix);
