/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <climits>
#include "lexer.h"

struct poly_decl_struct{
    struct polynomial_header* header;
    struct polynomial_body* body;
};

struct polynomial_header{
    std::string name;
    std::vector<struct id_list*> idList;
    int decl_line;

};

struct polynomial_body{
    struct term_list* body;
};

int order = 1;
struct id_list{
    std::string name;
    int order;
};

struct term_struct{
    int coefficient;
    std::vector<struct monomial*> mono_list;
};

struct term_list{
    struct term_struct* term;
    TokenType addOperator;
    struct term_list* next;
};

struct monomial{
    //start with 0.
    int order;
    int exponent;
};

struct argument{
    TokenType arg_type;
    int const_val = INT_MAX;
    int var_idx;
    struct poly_eval* polyEval;
};

struct poly_eval{
    std::string polyName;
    std::vector<struct argument*> args;
};

struct stmt{
    TokenType stmt_type;
    int param_idx;
    int variable;
    struct poly_eval* poly;
    struct stmt* next;
};

int eval_term(struct term_list* termList);
void eval_polynomial();
struct argument* get_last_arg_struct(struct poly_eval* p);
//void execute_program(struct stmt * start);
void assign_num();
int evaluate_polynomial(struct poly_eval* poly);
int eval_term(struct term_list* terms, std::vector<int> val);

class Parser {
private:
    LexicalAnalyzer lexer;

    stmt * parse_program();
    void parse_poly_decl_section();
    polynomial_header* parse_polynomial_header();
    void parse_id_list();
    std::string parse_polynomial_name();
    polynomial_body * parse_polynomial_body();
    term_list * parse_term_list();
    term_struct* parse_term();
    std::vector<monomial *> parse_monomial_list();
    monomial* parse_monomial();
    int parse_exponent();
    TokenType parse_add_operator();
    int parse_coefficient();
    stmt* parse_start();
    stmt* parse_statement_list();
    stmt * parse_statement();
    stmt * parse_poly_evaluation_statement();
    stmt * parse_input_statement();
    poly_eval * parse_polynomial_evaluation();
    void parse_argument_list();
    argument * parse_argument();
    void syntax_error();
    Token expect(TokenType expected_type);
    Token peek();
    void parse_inputs(stmt* st);
    poly_decl_struct* parse_poly_decl();
    void execute_program(stmt* st);

public:
    stmt * parse_input();

};


#endif

