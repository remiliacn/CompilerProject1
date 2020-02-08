/*
 * Copyright (C) Rida Bazzi, 2020
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */
#include <iostream>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <cmath>
#include "parser.h"
#include <string>
#include <cstddef>
#include <algorithm>
#include <utility>

using namespace std;
void Parser::syntax_error()
{
    cout << "SYNTAX ERROR !!!\n";
    exit(1);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

// this function simply checks the next token without
// consuming the input
// Written by Mohsen Zohrevandi
Token Parser::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}



// Parsing
vector<struct Var_struct> var;
vector<struct Poly_struct> poly;
vector<struct Term_struct> term;

Token t;

void Parser::parse_input(){
    parse_program();
    parse_inputs();
}

void Parser::parse_program(){
    parse_poly_decl_section();
    parse_start();
}

void Parser::parse_poly_decl_section(){
    parse_poly_decl();

    t = lexer.GetToken();

    if(t.token_type == POLY){
        parse_poly_decl_section();
    }
}

void Parser::parse_poly_decl(){
    t = lexer.GetToken();
    if(t.token_type == POLY){
        parse_polynomial_header();

        t = lexer.GetToken();
        if(t.token_type == EQUAL){
            parse_polynomial_body();
            t = lexer.GetToken();
            if(t.token_type != SEMICOLON){
                syntax_error();
            }
        }
    }
}

void Parser::parse_polynomial_body(){
    parse_term_list();
}

void Parser::parse_polynomial_header(){
    struct Poly_struct newPoly;
    newPoly.name = parse_polynomial_name();

    t = lexer.GetToken();
    if(t.token_type == LPAREN){
        id_list.clear();
        parse_id_list();
        newPoly.var = id_list;

        t = lexer.GetToken();
        if(t.token_type != RPAREN) {
            syntax_error();
        }
    }

    poly.push_back(newPoly);
}

void Parser::parse_id_list() {
    t = lexer.GetToken();
    if(t.token_type == ID){
        id_list.push_back(t.lexeme);
        t = lexer.GetToken();
        if(t.token_type == COMMA){
            parse_id_list();
        }

    } else{
        syntax_error();
    }
}

string Parser::parse_polynomial_name() {
    t = lexer.GetToken();
    if(t.token_type != ID){
        syntax_error();
    }

    return t.lexeme;
}

struct term_list* Parser::parse_term_list() {
    auto* termList = new term_list;
    struct term_list* newTerm;

    termList->term = parse_term();
    t = peek();
    if(t.token_type == PLUS || t.token_type == MINUS) {
        termList->op = parse_add_operator();
        newTerm = parse_term_list();
        termList->next = newTerm;
    }

    return termList;
}

struct term_struct* Parser::parse_term() {
    auto* termInfo = new term_struct;
    t = peek();
    if(t.token_type == NUM){
        t = lexer.GetToken();
        termInfo->coefficient = parse_coefficient();

        t = lexer.GetToken();
        if(t.token_type == ID) {
            monomial_list.clear();
            parse_monomial_list();
            termInfo->monoList = monomial_list;

        }else{
            termInfo->monoList = {};
        }

    } else if(t.token_type == ID){
        t = lexer.GetToken();
        parse_monomial_list();

    }else{
        syntax_error();
    }

    return termInfo;
}

void Parser::parse_monomial_list() {
    monomial_list.clear();
    t = lexer.GetToken();
    if(t.token_type == ID) {
        lexer.UngetToken(t);
        monomial_list.push_back(*parse_monomial());
    }
}

monomial* Parser::parse_monomial() {
    auto *monoInfo = new monomial;
    int idx = -1;
    t = lexer.GetToken();
    if(t.token_type == ID){
        id_list.clear();
        parse_id_list();

        if (!id_list.empty()){
            for (int i = 0; i < id_list.size(); i++){
                if (id_list[i] == t.lexeme){
                    idx = i;
                    monoInfo->order = i;
                    break;
                }
            }
        }

        if (idx != -1){
            monoInfo->exponent = parse_exponent();
        } else{
            syntax_error();
        }

    } else{
        syntax_error();
    }

    return monoInfo;
}

int Parser::parse_exponent() {
    t = lexer.GetToken();
    int num = INT_MAX;
    if(t.token_type == POWER){
        t = lexer.GetToken();
        if(t.token_type == NUM) {
            num = stoi(t.lexeme);

        } else{
            syntax_error();
        }
    }

    return num;
}

TokenType Parser::parse_add_operator() {
    t = lexer.GetToken();
    switch(t.token_type){
        case PLUS:
        case MINUS:
            return t.token_type;

        default:
            syntax_error();
    }
}

int Parser::parse_coefficient() {
    t = lexer.GetToken();
    if(t.token_type != NUM) {
        syntax_error();
    }

    return stoi(t.lexeme);
}

void Parser::parse_start() {
    t = lexer.GetToken();
    if(t.token_type == START){
        parse_statement_list();
    }
}

void Parser::parse_inputs() {
    t = lexer.GetToken();
    if(t.token_type == NUM){
        t = lexer.GetToken();
        if(t.token_type == NUM){
            parse_inputs();
        }
    } else{
        syntax_error();
    }
}

void Parser::parse_statement_list() {
    parse_statement();
}

void Parser::parse_statement() {
    Token tok = peek();

    if (tok.token_type == INPUT){
        parse_input_statement();

    } else if(tok.token_type == ID){
        parse_poly_evaluation_statement();

    } else{
        syntax_error();
    }
}

void Parser::parse_poly_evaluation_statement() {
    parse_polynomial_evaluation();
    t = lexer.GetToken();
    if(t.token_type == SEMICOLON){
        syntax_error();
    }
}

void Parser::parse_input_statement() {
    t = lexer.GetToken();
    auto* st = new stmt;
    if(t.token_type != INPUT){
        syntax_error();

    } else{
        t = lexer.GetToken();
        if(t.token_type != ID) {
            syntax_error();

        } else{
            st->stmt_type = true;
            st->var = INT_MAX;
            t = lexer.GetToken();
            if(t.token_type != SEMICOLON){
                syntax_error();
            }
        }
    }
}

void Parser::parse_polynomial_evaluation() {
    std::string varName = parse_polynomial_name();
    t = lexer.GetToken();
    if(t.token_type == LPAREN){
        parse_argument_list(varName);
        t = lexer.GetToken();
        if(t.token_type != RPAREN){
            syntax_error();
        }
    }
}


void Parser::parse_argument_list(std::string varName) {
    parse_argument();
    t = lexer.GetToken();
    if(t.token_type == COMMA){
        parse_argument_list(std::move(varName));
    } else{
        syntax_error();
    }
}

void Parser::parse_argument() {

}


int main()
{
    LexicalAnalyzer lexer;
    Token token;

    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}
