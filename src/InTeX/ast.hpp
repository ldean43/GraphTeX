#pragma once
#include <string>
#include <vector>

/*	Grammar
	eq ::=
		| Implicit {e e} // Implicit Equation i.e. f(x,y,z) = 0
		| Parametric {e e e} // Parametric Curve/Surface
	e ::=
		| Number {n}
		| Variable {v} 
		| Operator {o e e} 
		| Fraction {e e}
        | Sqrt {e e}
        | Log {e e} // Log with arbitrary base
        | Ln {e} // Natural Log
        | Lg {e} // Log base 2
        | Abs {e}
		| Trig {t e}
	n ::= float
	v ::= variable
	o ::= + | - | * | / | ^
	t ::= sin
		| cos
		| tan
		| sec
		| csc
		| cot
		| arcsin
		| arccos
		| arctan
        | arcsec
        | arccsc
        | arccot
		| sinh
		| cosh
		| tanh
*/

enum class Type {NUM, VAR, OP, OP_BRACE, FRAC, SQRT, LOG, LN, LG, ABS, TRIG, PARA, IMPL, UNDEF};

// base Expr with runtime polymorphism for copy and deconstructors
class Expr {
protected:
    Expr* exprCopy(const Expr* expr);
public:
    const Type type_;
    virtual Expr* copy() = 0;
	explicit Expr(const Type type) : type_(type) {}
    virtual ~Expr() {}
};

struct Equation {
	// base equation struct for inheritance
	const Type type_;

	explicit Equation(const Type type) : type_(type) {}
}  ;

struct Num : public Expr {
	const float value_;

    Expr* copy() override;
    explicit Num(const float value) : Expr(Type::NUM), value_(value) {}
};

struct Var : public Expr {
	const std::string value_;

    Expr* copy() override;
    explicit Var(const std::string &value) : Expr(Type::VAR), value_(value) {}
};

struct Op : public Expr {
	const char op_;
	const Expr* e1_;
	const Expr* e2_;

    Expr* copy() override;
    explicit Op(const char op, const Expr* e1, const Expr* e2) : Expr(Type::OP), op_(op), e1_(e1), e2_(e2) {}
    ~Op() override;
};

struct Frac : public Expr {
	const Expr* numerator_;
	const Expr* denominator_;

    Expr* copy() override;
    explicit Frac(const Expr* numerator, const Expr* denominator) :
    Expr(Type::FRAC), numerator_(numerator), denominator_(denominator) {}
    ~Frac() override;
};

struct Sqrt : public Expr {
    const Expr* root_;
    const Expr* e_;

    Expr* copy() override;
    ~Sqrt() override;
    explicit Sqrt(const Expr* root, const Expr* e) : Expr(Type::SQRT), root_(root), e_(e) {}
};

struct Log : public Expr {
	const Expr* base_;
	const Expr* e_;

    Expr* copy() override;
    explicit Log(const Expr* base, const Expr* e) : Expr(Type::LOG), base_(base), e_(e) {}
    ~Log() override;
};

struct Ln : public Expr {
	const Expr* e_;

    Expr* copy() override;
    explicit Ln(const Expr* e) : Expr(Type::LN), e_(e) {}
    ~Ln() override;
};

struct Lg : public Expr {
	const Expr* e_;

    Expr* copy() override;
    explicit Lg(const Expr* e) : Expr(Type::LG), e_(e) {}
    ~Lg() override;
};

struct Abs : public Expr {
	const Expr* e_;

    Expr* copy() override;
    explicit Abs(const Expr* e) : Expr(Type::ABS), e_(e) {}
    ~Abs() override;
};

struct Trig : public Expr {
	const std::string func_;
	const Expr* e_;

    Expr* copy() override;
    explicit Trig(const std::string& func, const Expr* e) : Expr(Type::TRIG), func_(func), e_(e) {}
    ~Trig() override;
};

struct Implicit : public Equation {
	const Expr* e1_;
	const Expr* e2_;

    explicit Implicit(const Expr* e1, const Expr* e2) : Equation(Type::IMPL), e1_(e1), e2_(e2) {}
};

struct Parametric : public Equation {
	const Expr* x_;
	const Expr* y_;
	const Expr* z_;

    explicit Parametric(const Expr* x, const Expr* y, const Expr* z) :
	Equation(Type::PARA), x_(x), y_(y), z_(z) {}
};