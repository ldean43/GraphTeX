#include "ast.hpp"
#include "utils.hpp"
#include "parser.hpp"
#include <stdexcept>

bool Parser::advance() {
    if (it_ < end_ - 1) {
        tok_ = *(++it_);
        return true;
    } else {
        ++it_;
        tok_ = "";
    }
    return false;
}

void Parser::safeAdvance(const std::string& op) {
    if (!advance()) {
        throw std::runtime_error 
        ("parsing error: unexpected end of input in " + op);
    }
    return;
}

void Parser::match(const std::string& op, const std::string& expected) {
    if (tok_ != expected) {
        if (it_ == end_) {
            throw std::runtime_error ("parsing error: unexpected end of input in "
            + op);
        }
        throw std::runtime_error ("parsing error: expected " + expected + 
        " after " + *(it_ - 1) + " in " + op + ", got " + tok_);
    }
}

Expr* Parser::popUnary (const std::string& op) {
    if (operand_.size() < 1) {
        throw std::runtime_error ("parsing error: insufficient operands for " + op);
    }
    Expr* e = operand_.top();
    operand_.pop();
    operator_.pop();
    return e;
}

std::pair<Expr*, Expr*> Parser::popBinary(std::string op) {
    if (operand_.size() < 2) {
        throw std::runtime_error ("parsing error: insufficent operands for " + op);
    }
    Expr* e2 = operand_.top();
    operand_.pop();
    Expr* e1 = operand_.top();
    operand_.pop();
    operator_.pop();
    return {e1, e2};
}

void Parser::popOperator() {
    std::string op = operator_.top();

    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "^") {
        std::pair<Expr*, Expr*> e = popBinary(op);
        operand_.push(new Op(op[0], e.first, e.second));
    } else if (op == "frac") {
        std::pair<Expr*, Expr*> e = popBinary(op);
        operand_.push(new Frac(e.first, e.second));
    } else if (op == "sqrt") {
        std::pair<Expr*, Expr*> e = popBinary(op);
        operand_.push(new Sqrt(e.first, e.second));
    } else if (op == "log") {
        std::pair<Expr*, Expr*> e = popBinary(op);
        operand_.push(new Log(e.first, e.second));
    } else if (op == "ln") {
        Expr* e = popUnary(op);
        operand_.push(new Ln(e));
    } else if (op == "lg") {
        Expr* e = popUnary(op);
        operand_.push(new Lg(e));
    } else if (trig_.count(op)) {
        Expr* e = popUnary(op);
        operand_.push(new Trig(op, e));
    }
    return;
}

Type Parser::parseNum() {
    operand_.push(new Num(stod(tok_)));
    advance();
    return Type::NUM;
}

Type Parser::parseVar() {
    operand_.push(new Var(tok_));
    advance();
    return Type::VAR;
}

Type Parser::parseOp() {
    while (!operator_.empty() &&
            (precedence_.at(operator_.top()) > precedence_.at(tok_) ||
            (precedence_.at(operator_.top()) == precedence_.at(tok_) && tok_ != "^"))) {
        popOperator();
    }
    operator_.push(tok_);
    safeAdvance(tok_);
    return Type::OP;
}
/*  
    Do not propogate Type::OP_BRACE within Frac as the render still maintains
    order of operations
*/
Type Parser::parseFrac() {
    const std::string frac("frac");

    operator_.push(tok_);
    safeAdvance(tok_);
    // parse numerator
    match(frac, "{");
    operator_.push(tok_);
    safeAdvance(frac);
    parseExpr();
    match(frac, "}");
    safeAdvance(frac);
    // parse denominator
    match("frac", "{");
    operator_.push(tok_);
    safeAdvance(frac);
    parseExpr();
    match("frac", "}");
    advance();
    return Type::FRAC;
}
/*
    Do not propogate Type::OP_BRACE within Sqrt as the render still maintains
    order of operations
*/
Type Parser::parseSqrt() {
    const std::string sqrt("sqrt");

    operator_.push(tok_);
    safeAdvance(sqrt);
    // Check for arbitrary root
    if (tok_ != "[") {
        operand_.push(new Num(2)); // if no custom root, choose 2
    // parse arbitrary root
    } else {
        operator_.push(tok_);
        safeAdvance(sqrt);
        parseExpr();
        match(sqrt, "]");
        safeAdvance(sqrt);
    }
    // parse argument
    match(sqrt, "{");
    operator_.push(tok_);
    safeAdvance(sqrt);
    parseExpr();
    match(sqrt, "}");
    advance();
    return Type::SQRT;
}
/*
    Do not propogate Type::OP within Log's base as the render still maintains
    order of operations
*/
Type Parser::parseLog() {
    Type type;
    const std::string log("log");

    operator_.push(tok_);
    safeAdvance(log);
    // check for arbitrary base
    if (tok_ != "_") { 
        operand_.push(new Num(10));
    // parse arbitrary base
    } else {
        safeAdvance(log);
        match(log, "{");
        operator_.push(tok_);
        safeAdvance(log);
        parseExpr();
        match(log, "}");
        safeAdvance(log);
    }
    // parse argument; throw std::runtime_error error on Type::OP or Type::OP_BRACE
    if (tok_ == "{" || tok_ == "(" || tok_ == "[") {
        operator_.push(tok_);
        if (tok_ == "{") {
            safeAdvance(log);
            type = parseExpr();
            if (type == Type::OP || type == Type::OP_BRACE) {
                throw std::runtime_error 
                ("parsing error: log argument contains operation in braces;"
                " misleading order of operations in render, use parentheses instead");
            }
            match(log, "}");
        } else if (tok_ == "(") {
            safeAdvance(log);
            parseExpr();
            match(log, ")");
        } else if (tok_ == "[") {
            safeAdvance(log);
            parseExpr();
            match(log, "]");
        }
    } else {
        throw std::runtime_error ("parsing error: expected {, (, or [ after log");
    }
    advance();
    return Type::LOG;
}

Type Parser::parseLnLg() {
    Type type;
    const std::string func = tok_;

    operator_.push(tok_);
    safeAdvance(tok_);
    // parse argument, throw std::runtime_error error on Type::OP or Type::OP_BRACE
    if (tok_ == "{" || tok_ == "(" || tok_ == "[") {
        operator_.push(tok_);
        if (tok_ == "{") {
            safeAdvance(func);
            type = parseExpr();
            if (type == Type::OP || type == Type::OP_BRACE) {
                throw std::runtime_error 
                ("parsing error: " + func + " argument contains operation in braces;"
                " misleading order of operations in render, use parentheses or"
                " brackets instead");
            }
            match(func, "}");
        } else if (tok_ == "(") {
            safeAdvance(func);
            parseExpr();
            match(func, ")");
        } else if (tok_ == "[") {
            safeAdvance(func);
            parseExpr();
            match(func, "]");
        }
    } else {
         throw std::runtime_error 
         ("parsing error: expected { or ( or [ after " + func);
    }
    advance();
    return Type::LN;
}

Type Parser::parseAbs() {
    // Push left| onto stack
    operator_.push(tok_);
    safeAdvance(tok_);
    parseExpr();
    // Check for required right|
    match("absolute value", "right|");
    if (operand_.size() < 1) {
        throw std::runtime_error("parsing error: insufficient operands in absolute value");
    }
    Expr* e = operand_.top();
    operand_.pop();
    operand_.push(new Abs(e));
    advance();
    return Type::ABS;
}

Type Parser::parseTrig() {
    Type type;
    const std::string func = tok_;

    operator_.push(func);
    safeAdvance(func);
    // parse argument, throw std::runtime_error error on Type::OP or Type::OP_BRACE
    if (tok_ == "{" || tok_ == "(" || tok_ == "[") {
        operator_.push(tok_);
        if (tok_ == "{") {
            safeAdvance(func);
            type = parseExpr();
            if (type == Type::OP || type == Type::OP_BRACE) {
                throw std::runtime_error 
                (  "parsing error: " + func + " argument contains operation in braces;"
                " misleading order of operations in render, use parentheses instead");
            }
            match(func, "}");
        } else if (tok_ == "(") {
            safeAdvance(func);
            parseExpr();
            match(func, ")");
        } else if (tok_ == "[") {
            safeAdvance(func);
            parseExpr();
            match(func, "]");
        } 
    } else {
        throw std::runtime_error 
        ("parsing error: expected { or ( or [ after " + func);
    }
    advance();
    return Type::TRIG;
}

Type Parser::parseStartDelim() {
    Type type = Type::UNDEF;

    operator_.push(tok_);
    if (tok_ == "(" ) {
        safeAdvance("(");
        type = parseExpr();
        match("parenthesis", ")");
    // begin propagation of Type::OP_BRACE
    } else if (tok_ == "{") {
        safeAdvance("{");
        type = parseExpr();
        type = type == Type::OP ? Type::OP_BRACE : type;
        match("braces", "}");
    } else if (tok_ == "[") {
        safeAdvance("[");
        type = parseExpr();
        match("brackets", "]");
    }
    advance();
    return type;
}

void Parser::parseEndDelim() {
    if (tok_ == ")") {
        while (!(operator_.empty() || operator_.top() == "(")) {
            if (operator_.top() == "{" || operator_.top() == "[" || 
                operator_.top() == "left|") {
                break;
            }
            popOperator();
        }
        if (operator_.empty() || operator_.top() != "(") {
            throw std::runtime_error ("parsing error: mismatched (");
        }
        operator_.pop();
    } else if (tok_ == "}") {
        while (!(operator_.empty() || operator_.top() == "{")) {
            if (operator_.top() == "(" || operator_.top() == "[" || 
                operator_.top() == "left|") {
                break;
            }
            popOperator();
        }
        if (operator_.empty() || operator_.top() != "{") {
            throw std::runtime_error ("parsing error: mismatched {");
        }
        operator_.pop();
    } else if (tok_ == "]") {
        while (!(operator_.empty() || operator_.top() == "[")) {
            if (operator_.top() == "(" || operator_.top() == "{" || 
                operator_.top() == "left|") {
                break;
            }
            popOperator();
        }
        if (operator_.empty() || operator_.top() != "[") {
            throw std::runtime_error ("parsing error: mismatched [");
        }
        operator_.pop();
    } else if (tok_ == "right|") {
        while (!(operator_.empty() || operator_.top() == "left|")) {
            if (operator_.top() == "(" || operator_.top() == "[" || 
                operator_.top() == "{") {
                break;
            }
            popOperator();
        }
        if (operator_.empty() || operator_.top() != "left|") {
            throw std::runtime_error ("parsing error: mismatched left|");
        }
        operator_.pop();
    }
    return;
}
/* 
    type is used to propogate prescence of Type::OP_BRACE within braces as it 
    can lead to misleading order of operations in LaTeX render. 
    Only propogate Type::OP if it is present in braces, otherwise return the 
    type of the last expression parsed.

    For functions that don't propogate the presence of Type::OP_BRACE, their
    render still maintains order of operations so there is no need to throw an error.
    If there is still a nested operation in braces within the argument, 
    it will be caught by a deeper recursive call.
*/
Type Parser::parseExpr() {
    Type type = Type::UNDEF, next_type = Type::UNDEF;
    if (is_float(tok_)) {
        type = parseNum();
    } else if (is_var(tok_)) {
        type = parseVar();
    } else if (tok_ == "+" || tok_ == "-" || tok_ == "*" || tok_ == "/" || tok_ == "^") {
        type = parseOp();
    } else if (tok_ == "frac") {
        type = parseFrac();
    } else if (tok_ == "sqrt") {
        type = parseSqrt();
    } else if (tok_ == "log") {
        type = parseLog();
    } else if (tok_ == "ln" || tok_ == "lg") {
        type = parseLnLg();
    } else if (trig_.count(tok_)) {
        type = parseTrig();
    } else if (tok_ == "left|") {
        type = parseAbs();
    } else if (tok_ == "(" || tok_ == "{" || tok_ == "[") {
        type = parseStartDelim();
    } else if (tok_ == ")" || tok_ == "}" || tok_ == "]" || tok_ == "right|") {
        parseEndDelim();
        return Type::UNDEF;
    } else {
        throw std::runtime_error ("parsing error: invalid token " + tok_);
    }
    if (it_ != end_) {
        next_type = parseExpr();
    }
    if ((type == Type::OP && next_type == Type::OP_BRACE) ||
        (type == Type::OP_BRACE && next_type == Type::OP)) {
        throw std::runtime_error ("parsing error: adjacent operations in braces");
    }
    return (next_type == Type::OP || next_type == Type::OP_BRACE) ? next_type : type;
}

Parser::Parser (const std::vector<std::string> tokens) : tokens_(tokens), it_(tokens_.begin()),
end_(tokens_.end()) {
    tok_ = *it_;
}

Expr* Parser::parse() {
    if (!parsed) {
        parseExpr();
        if (operand_.empty()) {
            throw std::runtime_error("parsing error: no operands provided");
        }
        while (!operator_.empty()) {
            popOperator();
        }
        parsed = true;
    }
    if (operand_.size() > 1) {
        throw std::runtime_error("parsing error: too many operands");
    }
    return operand_.top();
}

Parser::~Parser() {
    while (!operand_.empty()) {
        delete operand_.top();
        operand_.pop();
    }
}
// Exponentiation with right associativity
const std::unordered_map<std::string, unsigned int> Parser::precedence_ = {
    {"sin", 5}, {"cos", 5}, {"tan", 5}, {"csc", 5}, {"sec", 5}, {"cot", 5},
    {"arcsin", 5}, {"arccos", 5}, {"arctan", 5}, {"arcsec", 5}, {"arccsc", 5}, 
    {"arccot", 5}, {"sinh", 5}, {"cosh", 5}, {"tanh", 5}, {"frac", 5}, 
    {"sqrt", 5}, {"ln", 5}, {"log", 5}, {"lg", 5}, {"^", 4}, {"*", 3}, {"/", 3},
    {"+", 2}, {"-", 2}, {"(", 1}, {"{", 1}, {"[", 1}, {"left|", 1}
};

const std::unordered_set<std::string> Parser::trig_ = {
    "sin", "cos", "tan", "csc", "sec", "cot",
    "arcsin", "arccos", "arctan", "arcsec",
    "arccsc", "arccot", "sinh", "cosh", "tanh"
};
