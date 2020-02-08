/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <map>
#include "lexer.h"

class Parser {
    private:
        LexicalAnalyzer lexer;
        Token expect(TokenType expected_type);
        Token peek();

    public:

        void syntax_error();

        int errorCode = 0;
        std::vector<int> errorLine1;
        std::vector<int> errorLine2;
        std::vector<int> errorLine3;
        std::vector<int> errorLine4;
        std::map<std::string, int> inputOrder;
        std::map<std::string, std::map<std::string, int>> packedNum;




};

#endif

