#pragma once
#include "ast.hpp"
#include <stack>
#include <unordered_map>
#include <unordered_set>

// Recursive descenct parser using the shunting yard algorithm
class Parser {
    private:
        // Static members
        static const std::unordered_map<std::string, unsigned int> precedence_;
        static const std::unordered_set<std::string> trig_;
        // Shunting yard stacks
        std::stack<Expr*> operand_;
        std::stack<std::string> operator_;
        // Iterators
        std::vector<std::string> tokens_;
        std::vector<std::string>::const_iterator it_, end_;
        // Temporary parsing
        std::string tok_;
        bool parsed = false;

        bool advance();

        void safeAdvance(const std::string& op);

        void match(const std::string& op, const std::string& expected);

        Expr* popUnary(const std::string& op);

        std::pair<Expr*, Expr*> popBinary(std::string op);

        void popOperator();

        Type parseNum();

        Type parseVar();

        Type parseOp();

        Type parseFrac();

        Type parseSqrt();

        Type parseLog();

        Type parseLnLg();

        Type parseAbs();

        Type parseTrig();

        Type parseStartDelim();

        void parseEndDelim();

        Type parseExpr();

    public:
        explicit Parser(const std::vector<std::string> tokens);
        ~Parser();
        Expr* parse();
};