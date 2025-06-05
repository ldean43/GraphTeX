#include "utils.hpp"
#include <iostream>

bool is_float(std::string str) {
    bool has_decimal = false;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '.') { 
            if (has_decimal) {
                return false;
            } else {
                has_decimal = true;
            }
        } else if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool is_var(std::string str) {
    if (str.length() == 1 && isalpha(str[0])) {
        return true;
    }
    return false;
}

void print_ast(const Expr* expr, const std::string prefix) {
    switch (expr->type_) {
        case Type::NUM:
            std::cout << std::to_string(((Num*)(expr))->value_) << std::endl;
            break;
        case Type::VAR:
            std::cout << ((Var*)(expr))->value_ << std::endl;
            break;
        case Type::OP:
            std::cout << ((Op*)expr)->op_ << std::endl;
            std::cout << prefix << "├── " << "e1: ";
            if (((Op*)expr)->e1_->type_ != Type::NUM && ((Op*)expr)->e1_->type_ != Type::VAR) {
                print_ast(((Op*)expr)->e1_, prefix + "│   ");
            } else {
                print_ast(((Op*)expr)->e1_, prefix);
            }
            std::cout << prefix << "└── " << "e2: ";
            if (((Op*)expr)->e2_->type_ != Type::NUM && ((Op*)expr)->e2_->type_ != Type::VAR) {
                print_ast(((Op*)expr)->e2_, prefix + "    ");
            } else {
                print_ast(((Op*)expr)->e2_, prefix);
            }
            break;
        case Type::FRAC:
            std::cout << "frac" << std::endl;
            std::cout << prefix << "├── " << "numerator: ";
            if (((Frac*)expr)->numerator_->type_ != Type::NUM && ((Frac*)expr)->numerator_->type_ != Type::VAR) {
                print_ast(((Frac*)expr)->numerator_, prefix + "│   ");
            } else {
                print_ast(((Frac*)expr)->numerator_, prefix);
            }
            std::cout << prefix << "└── " << "denominator: ";
             if (((Frac*)expr)->denominator_->type_ != Type::NUM && ((Frac*)expr)->denominator_->type_ != Type::VAR) {
                print_ast(((Frac*)expr)->denominator_, prefix + "    ");
            } else {
                print_ast(((Frac*)expr)->denominator_, prefix);
            }
            break;
        case Type::SQRT:
            std::cout << "sqrt" << std::endl;
            std::cout << prefix << "├── " << "root: ";
            if (((Sqrt*)expr)->root_->type_ != Type::NUM && ((Sqrt*)expr)->root_->type_ != Type::VAR) {
                print_ast(((Sqrt*)expr)->root_, prefix + "│   ");
            } else {
                print_ast(((Sqrt*)expr)->root_, prefix);
            }
            std::cout << prefix << "└── " << "e: ";
            if (((Sqrt*)expr)->e_->type_ != Type::NUM && ((Sqrt*)expr)->e_->type_ != Type::VAR) {
                print_ast(((Sqrt*)expr)->e_, prefix + "    ");
            } else {
                print_ast(((Sqrt*)expr)->e_, prefix);
            }
            break;
        case Type::LOG:
            std::cout << "log" << std::endl;
            std::cout << prefix << "├── " << "base: ";
            if (((Log*)expr)->base_->type_ != Type::NUM && ((Log*)expr)->base_->type_ != Type::VAR) {
                print_ast(((Log*)expr)->base_, prefix + "    ");
            } else {
                print_ast(((Log*)expr)->base_, prefix);
            }
            std::cout << prefix << "└── " << "e: ";
            if (((Log*)expr)->e_->type_ != Type::NUM && ((Log*)expr)->e_->type_ != Type::VAR) {
                print_ast(((Log*)expr)->e_, prefix + "    ");
            } else {
                print_ast(((Log*)expr)->e_, prefix);
            }
            break;
        case Type::LN:
            std::cout << "ln" << std::endl;
            std::cout << prefix << "└── " << "e: ";
            if (((Ln*)expr)->e_->type_ != Type::NUM && ((Ln*)expr)->e_->type_ != Type::VAR) {
                print_ast(((Ln*)expr)->e_, prefix + "    ");
            } else {
                print_ast(((Ln*)expr)->e_, prefix);
            }
            break;
        case Type::LG:
            std::cout << "lg" << std::endl;
            std::cout << prefix << "└── " << "e: ";
            if (((Lg*)expr)->e_->type_ != Type::NUM && ((Lg*)expr)->e_->type_ != Type::VAR) {
                print_ast(((Lg*)expr)->e_, prefix + "    ");
            } else {
                print_ast(((Lg*)expr)->e_, prefix);
            }
            break;
        case Type::TRIG:
            std::cout << ((Trig*)expr)->func_ << std::endl;
            std::cout << prefix << "└── " << "e: ";
            if (((Trig*)expr)->e_->type_ != Type::NUM && ((Trig*)expr)->e_->type_ != Type::VAR) {
                print_ast(((Trig*)expr)->e_, prefix + "    ");
            } else {
                print_ast(((Trig*)expr)->e_, prefix);
            }
            break;
        case Type::ABS:
            std::cout << "abs" << std::endl;
            std::cout << prefix << "└── " << "e: ";
            if (((Abs*)expr)->e_->type_ != Type::NUM && ((Abs*)expr)->e_->type_ != Type::VAR) {
                print_ast(((Abs*)expr)->e_, prefix + "    ");
            } else {
                print_ast(((Abs*)expr)->e_, prefix);
            }
            break;
        default:
            throw std::runtime_error ("printing error: invalid expression type");
    }
}
