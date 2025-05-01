#include "ast.hpp"
#include <iostream>

Expr* Expr::exprCopy(const Expr* expr) {
    switch (expr->type_) {
    case Type::NUM:
        return new Num(((Num*)expr)->value_);
    case Type::VAR:
        return new Var(((Var*)expr)->value_);
    case Type::OP:
        return new Op(((Op*)expr)->op_, 
        exprCopy(((Op*)expr)->e1_), 
        exprCopy(((Op*)expr)->e2_));
    case Type::FRAC:
        return new Frac
        (exprCopy(((Frac*)expr)->numerator_), 
        exprCopy(((Frac*)expr)->denominator_));
    case Type::SQRT:
        return new Sqrt
        (exprCopy(((Sqrt*)expr)->root_), 
        exprCopy(((Sqrt*)expr)->e_));
    case Type::LOG:
        return new Log
        (exprCopy(((Log*)expr)->base_), 
        exprCopy(((Log*)expr)->e_));
    case Type::LN:
        return new Ln(exprCopy(((Ln*)expr)->e_));
    case Type::LG:
        return new Lg(exprCopy(((Lg*)expr)->e_));
    case Type::TRIG:
        return new Trig
        (((Trig*)expr)->func_,
        exprCopy(((Trig*)expr)->e_));
    case Type::ABS:
        return new Abs(exprCopy(((Abs*)expr)->e_));
    default:
        throw std::runtime_error ("copy error: invalid expression");
    }
}

Expr* Num::copy() {
    return new Num(this->value_);
}

Expr* Var::copy() {
    return new Var(this->value_);
}

Expr* Op::copy() {
    return new Op(this->op_, exprCopy(this->e1_), exprCopy(this->e2_));
}

Expr* Frac::copy() {
    return new Frac(exprCopy(this->numerator_), exprCopy(this->denominator_));
}

Expr* Sqrt::copy() {
    return new Sqrt(exprCopy(this->root_), exprCopy(this->e_));
}

Expr* Log::copy() {
    return new Log(exprCopy(this->base_), exprCopy(this->e_));
}

Expr* Ln::copy() {
    return new Ln(exprCopy(this->e_));
}

Expr* Lg::copy() {
    return new Lg(exprCopy(this->e_));
}

Expr* Abs::copy() {
    return new Abs(exprCopy(this->e_));
}

Expr* Trig::copy() {
    return new Trig(this->func_, exprCopy(this->e_));
}

Op::~Op() {
    delete(this->e1_);
    delete(this->e2_);
}

Frac::~Frac() {
    delete(this->numerator_);
    delete(this->denominator_);
}

Sqrt::~Sqrt() {
    delete(this->root_);
    delete(this->e_);
}

Log::~Log() {
    delete(this->base_);
    delete(this->e_);
}

Ln::~Ln() {
    delete(this->e_);
}

Lg::~Lg() {
    delete(this->e_);
}

Abs::~Abs() {
    delete(this->e_);
}

Trig::~Trig() {
    delete (this->e_);
}




