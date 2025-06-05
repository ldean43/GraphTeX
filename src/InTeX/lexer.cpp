#include "ast.hpp"
#include "utils.hpp"
#include "lexer.hpp"
#include <stdexcept>
#include <unordered_set>

void Lexer::advance() {
    if (it_ < end_ - 1) {
        c_ = *(++it_);
    } else {
        ++it_;
        c_ = '\0';
    }
    return;
}

// Handles \sqrt[]{}, \frac{}{}, \log_{}{}
void Lexer::implicitMultHelper(std::vector<std::string>& new_toks, unsigned int& i) {
    std::string next;

    if (i < tokens_.size()) { 
        new_toks.push_back(tokens_[i]); // Push first opening brace
    }
    implicitMult(new_toks, ++i); // Retokenize ...}
    if (++i < tokens_.size()) { 
        new_toks.push_back(tokens_[i]); // Push second opening brace
    }
    implicitMult(new_toks, ++i); // Retokenize ...}
    if (i < tokens_.size() - 1 && tokens_[i] == "}") {
        next = tokens_[i + 1];
        if (is_float(next) || is_var(next) ||
            next == "{" || next == "(" || next == "[" || 
            (valid_cmds_.count(next) && next != "right|")) {
            new_toks.push_back("*");
        }
        implicitMult(new_toks, ++i); // Retokenize remaining tokens
    }
    return;
}

// Retokenizes for cases with implicit multiplication i.e. 5(2), 5x
void Lexer::implicitMult(std::vector<std::string>& new_toks, unsigned int& i) {
    std::string next;

    if (i < tokens_.size()) { 
        new_toks.push_back(tokens_[i]); // Push current token
    } else { 
        return; 
    }

    if (tokens_[i] == "}" || tokens_[i] == ")" || tokens_[i] == "]" || tokens_[i] == "right|") {
        return;
    } else if (tokens_[i] == "frac") {
        implicitMultHelper(new_toks, ++i); 
    } else if (tokens_[i] == "{" || tokens_[i] == "(" || tokens_[i] == "[") {
        implicitMult(new_toks, ++i); // Retokenize ...}
        if (i < tokens_.size() - 1) {
            next = tokens_[i + 1];
            if (is_float(next) || is_var(next) || 
                next == "{" || next == "(" || next == "[" || 
                (valid_cmds_.count(next) && next != "right|")) {
                new_toks.push_back("*");
            }
        } 
    } else if (i < tokens_.size() - 1) {
        next = tokens_[i + 1];
        if (is_float(tokens_[i])) { // Implicit mult after number
            if (is_var(next) || 
                next == "{" || next == "(" || next == "[" ||
                (valid_cmds_.count(next) && next != "right|")) {
                new_toks.push_back("*");
            }
        } else if (tokens_[i] == "log") { // Implicit mult after log with arbitrary base
            if (next == "_") { 
                new_toks.push_back(tokens_[++i]); 
                implicitMultHelper(new_toks, ++i);
            }
        } else if (tokens_[i] == "sqrt") { // Implicit mult after sqrt with arbitrary root
            if (next == "[") {
                implicitMultHelper(new_toks, ++i);
            }
        } else if (is_var(tokens_[i])) { // Implicit mult after variable
            if (is_float(next) || is_var(next) || 
                next == "{" || next == "(" || next == "[" ||
                (valid_cmds_.count(next) && next != "right|")) {
                new_toks.push_back("*");
            }
        }
    }
    implicitMult(new_toks, ++i); // Retokenize remaining tokens
    return;
}

void Lexer::lexHelper() {
    std::string command;
    bool negative = false;

    while (it_ != end_) {

        if (isspace(c_)) { // Ignore whitespace
            advance();
        } else if (c_ == '\\') { // Handle commands, they always start with \ 
            advance();
            while (it_ != end_ && (isalpha(c_) || c_ == '|')) {
                command += c_;
                advance();
            }
            if (valid_cmds_.count(command)) {
                tokens_.push_back(command);
                command = "";
            } else {
                throw std::runtime_error("lexing error: invalid operator " + command);
            }
        }
        else if (valid_syms_.count(c_)) { // Handle operators and delimeters
            unsigned int n = 0;
            if (c_ == '-' || c_ == '+') { // For inputs like ---3 = -3 or ++-3 = -3
                while (it_ != end_ && (c_ == '-' || c_ == '+')) {
                    if (c_ == '-') { n++; }
                    advance();
                }
                if (n % 2) {
                    if (tokens_.empty() || tokens_.back() == "{" || tokens_.back() == "(" || tokens_.back() == "[") {
                        tokens_.push_back("0");
                    }
                    tokens_.push_back("-");
                } else {
                    if (!(tokens_.empty() || tokens_.back() == "{" || tokens_.back() == "(" || tokens_.back() == "[")) {
                        tokens_.push_back("+");
                    }
                }
            } else {
                tokens_.push_back(std::string(1, c_));
                advance();
            }
        }
        else if (isalpha(c_)) { // Handle single character variables
            tokens_.push_back(std::string(1,c_));
            advance();
        }
        else if (std::isdigit(c_) || c_ == '.') { // Handle numbers
            std::string number(1,c_);
            bool has_decimal = c_ == '.';
            advance();
            while (it_ != end_ && (std::isdigit(c_) || c_ == '.')) {
                if (c_ == '.') {
                    if (has_decimal) {
                        throw std::runtime_error ("lexing error: invalid number, two decimals");
                    } else {
                        has_decimal = true;
                    }
                }
                number += c_;
                advance();
            }
            tokens_.push_back(number);
        } else {
            throw std::runtime_error("lexing error: invalid token");
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
