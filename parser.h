/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"
#include "map"
#include "vector"

std::vector<std::string> id_list;
int inputIdx;

struct term_struct{
    int coefficient;
    std::vector<struct monomial> monoList;
};

struct term_list{
    struct term_struct* term;
    TokenType op;
    struct term_list* next;
};

struct monomial{
    int order;
    int exponent;
};

std::vector<struct monomial> monomial_list;

struct stmt{
    //stmt_type value
    //true -> input
    //false -> poly_eval
    bool stmt_type;
    struct poly_eval;
    int var;
};

std::vector<struct stmt> statement_list;

struct poly_eval{
    std::string refName;
    struct args;
};


std::vector<struct poly_eval> poly_eval_list;

struct args{
    TokenType type = {};
    int var = INT_MAX;
    int idx = -1;
    struct poly_eval;
};

std::map<std::string, std::vector<struct args>> argumentMap;
std::vector<struct args*> argList;
std::vector<std::string> polyList;
std::vector<std::string> inputList;

struct Var_struct{
    std::string name;
    int val;
};

struct Poly_struct{
    std::string name;
    struct id_list;
    std::vector<std::string> var;

};

class Parser {
private:
    LexicalAnalyzer lexer;

    void syntax_error();
    Token expect(TokenType expected_type);
    Token peek();
    void parse_inputs();
    void parse_poly_decl();
    void parse_input();
    void parse_program();
    void parse_poly_decl_section();
    void parse_polynomial_header();
    void parse_id_list();
    std::string parse_polynomial_name();
    void parse_polynomial_body();
    struct term_list* parse_term_list();
    struct term_struct* parse_term();
    void parse_monomial_list();
    monomial* parse_monomial();
    int parse_exponent();
    TokenType parse_add_operator();
    int parse_coefficient();
    void parse_start();
    void parse_statement_list();
    void parse_statement();
    void parse_poly_evaluation_statement();
    void parse_input_statement();
    void parse_polynomial_evaluation();
    void parse_argument_list(std::string varName);
    args * parse_argument(std::string varName);
    void execute_program();
};


#endif
