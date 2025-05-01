#pragma once
#include <string>
#include <vector>
#include <unordered_set>

class Lexer {
    private:
        // Static members
        const static std::unordered_set<std::string> valid_cmds_;
        const static std::unordered_set<char> valid_syms_;
        // Tokenized input
        std::vector<std::string> tokens_;
        // Iterators
        const std::string input_;
        std::string::const_iterator it_, end_;
        // Temporary parsing
        char c_;

        void advance();

        void implicitMultHelper(std::vector<std::string>& new_toks, unsigned int& index);

        void implicitMult(std::vector<std::string>& new_toks, unsigned int& index);

        void lexHelper();
    public:
        explicit Lexer(const std::string input);

        std::vector<std::string> lex();
};