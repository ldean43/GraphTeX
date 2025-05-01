#include "ast.hpp"
#include "utils.hpp"
#include "lexer.hpp"
#include <stdexcept>
#include <unordered_set>

void Lexer::advance() {
    if (it_ != (end_ - 1)) {
        c_ = *(++it_);
    } else {
        ++it_;
    }
    return;
}

void Lexer::implicitMultHelper(std::vector<std::string>& new_toks, unsigned int& i) {
    std::string next;

    if (i < tokens_.size()) { new_toks.push_back(tokens_[i]); }
    implicitMult(new_toks, ++i);
    if (++i < tokens_.size()) { new_toks.push_back(tokens_[i]); }
    implicitMult(new_toks, ++i);
    if (i < tokens_.size() && tokens_[i] == "}") {
        if (i < tokens_.size() - 1) {
            next = tokens_[i + 1];
            if (is_float(next) || is_var(next) || next == "{" || next == "(" ||
                next == "[" || (valid_cmds_.count(next) && next != "right|")) {
                new_toks.push_back("*");
            }
        }
        implicitMult(new_toks, ++i);
    }
    return;
}

// Retokenizes for cases with implicit multiplication i.e. 5(2), 5x
void Lexer::implicitMult(std::vector<std::string>& new_toks, unsigned int& i) {
    std::string next;

    if (i < tokens_.size()) { 
        new_toks.push_back(tokens_[i]);
    } else { return; }

    if (tokens_[i] == "}" || tokens_[i] == ")" || tokens_[i] == "]" ||
        tokens_[i] == "right|") {
        return;
    } else if (tokens_[i] == "frac") {
        implicitMultHelper(new_toks, ++i); 
    } else if (tokens_[i] == "{" || tokens_[i] == "(" || tokens_[i] == "[") {
        // checking inside braces
        implicitMult(new_toks, ++i);
        if (i < tokens_.size() - 1) {
            next = tokens_[i + 1];
            if (is_float(next) || is_var(next) || next == "{" || next == "(" ||
                next == "[" || (valid_cmds_.count(next) && next != "right|")) {
                new_toks.push_back("*");
            }
        } 
    } else if (i < tokens_.size() - 1) {
        next = tokens_[i + 1];
        // implicit mult after num?
        if (is_float(tokens_[i])) {
            if (is_var(next) || next == "{" || next == "(" || next == "[" ||
                (valid_cmds_.count(next) && next != "right|")) {
                new_toks.push_back("*");
            }
        // handle special case for log with arbitrary base
        } else if (tokens_[i] == "log") {
            if (next == "_") { 
                new_toks.push_back(tokens_[++i]); 
                implicitMultHelper(new_toks, ++i);
            }
        } else if (tokens_[i] == "sqrt") {
            if (next == "[") {
                implicitMultHelper(new_toks, ++i);
            }
        // handle implicit multiplaction following variable or delim
        } else if (is_var(tokens_[i])) {
            if (is_float(next) || is_var(next) || next == "{" || next == "(" || next == "[" ||
                (valid_cmds_.count(next) && next != "right|")) {
                new_toks.push_back("*");
            }
        }
    }
    implicitMult(new_toks, ++i);
    return;
}

void Lexer::lexHelper() {
    std::string command;

    while (it_ != end_) {

        // Handle whitespace
        if (isspace(c_)) {
            advance();
            continue;
        }
        // operator_s
        if (c_ == '\\') {
            advance();
            while (it_ != end_ && (isalpha(c_) || c_ == '|')) {
                command += c_;
                advance();
            }
            // Check if command is valid
            if (valid_cmds_.count(command)) {
                tokens_.push_back(command);
                command = "";
                continue;
            } else {
                throw std::runtime_error 
                ("lexing error: invalid operator " + command);
            }
        }
        /* command will typically be followed by a symbol, symbol logic
        immediately after operator_ logic to optimize time */
        if (valid_syms_.count(c_)) {
            if (c_ == '-') {
                if (tokens_.empty() || (!is_float(tokens_.back()) &&
                    !is_var(tokens_.back()))) {
                    tokens_.push_back("0");
                }
            }
            tokens_.push_back(std::string(1, c_));
            advance();
            continue;
        }
        // Variables
        if (isalpha(c_)) {
            tokens_.push_back(std::string(1,c_));
            advance();
            continue;
        }
        // Numbers
        if (std::isdigit(c_) || c_ == '.') {
            std::string number(1,c_);
            bool has_decimal = c_ == '.';
            advance();
            while (it_ != end_ && (std::isdigit(c_) || c_ == '.')) {
                // Check for two decimals
                if (c_ == '.') { 
                    if (has_decimal) {
                        throw std::runtime_error ("lexing error: invalid input");
                    } else {
                        has_decimal = true;
                    }
                }
                // Append digit to number
                number += c_;
                advance();
            }
            tokens_.push_back(number);
        }
    }
    return;
}

std::vector<std::string> Lexer::lex() {
    std::vector<std::string> new_toks;
    unsigned int i = 0;
    lexHelper();
    implicitMult(new_toks, i);
    tokens_ = new_toks;
    return tokens_;
}

Lexer::Lexer(const std::string input) : input_(input), it_(input_.begin()), end_(input_.end()) {
    c_ = *it_;
}

const std::unordered_set<std::string> Lexer::valid_cmds_ = {
    "sin", "cos", "tan", "csc", "sec", "cot",
    "arcsin", "arccos", "arctan", "arcsec",
    "arccsc", "arccot", "sinh", "cosh", "tanh",
    "frac", "sqrt", "ln", "log", "lg", "right|", 
    "left|"
};

const std::unordered_set<char> Lexer::valid_syms_ = {
    '+', '-', '*', '/', '^', '=', '(', ')', '{', '}', '[', ']', '_'
};
