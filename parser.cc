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
#include <cstdlib>
#include <map>
#include "parser.h"
#include <vector>
#include <cmath>

using namespace std;
Token t;

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
int varIdx = 0;
//save order of params appear after INPUT
map<string, int> inputMap;
int memory[100];
vector<int> inputNum;
vector<struct argument*> argVec;
vector<struct id_list*> idVec;
vector<struct poly_decl_struct*> polyVec;
vector<struct stmt*> stmtVec;
vector<monomial*> monoVec;
vector<int> calcResults;


struct stmt* Parser::parse_input(){
    struct stmt* st = parse_program();
    order = 0;
    parse_inputs(st);
    return st;
}

struct stmt* Parser::parse_program(){
    parse_poly_decl_section();
    struct stmt* st = parse_start();
    return st;
}

//append POLY list
void Parser::parse_poly_decl_section(){
    poly_decl_struct* newPoly = parse_poly_decl();
    polyVec.push_back(newPoly);
    t = peek();
    if(t.token_type == POLY){
        parse_poly_decl_section();
    }
}

//polynomial
struct poly_decl_struct* Parser::parse_poly_decl(){
    struct poly_decl_struct* poly = new poly_decl_struct;
    expect(POLY);
    poly->header = parse_polynomial_header();

    expect(EQUAL);
    poly->body = parse_polynomial_body();
    expect(SEMICOLON);
    return poly;
}

//X^2+2x+1
struct polynomial_body* Parser::parse_polynomial_body(){
    struct polynomial_body* body = new polynomial_body;
    body->body = parse_term_list();
    return body;
}


//F, F(X), F(X,Y)
struct polynomial_header* Parser::parse_polynomial_header(){
    struct polynomial_header* header = new polynomial_header;
    //vector<struct id_list*> list;
    struct id_list* param = new id_list;
    param->name = "x";
    param->order = 0;
    //list.push_back(param);
    idVec.clear();


    header->name = parse_polynomial_name();
    t = peek();
    //POLY F(X,Y) = X^2 + Y;
    //POLY G(X) = X^2+5;

    if(t.token_type == LPAREN){
        free(param);
        order = 0;
        t = lexer.GetToken();
        parse_id_list();

        t = expect(RPAREN);

    }else{
        idVec.push_back(param);
    }

    header->idList = idVec;
    return header;
}


void Parser::parse_id_list() {
    struct id_list* idList = new id_list;
    t = lexer.GetToken();
    if (t.token_type == ID){
        idList->name = t.lexeme;
        idList->order = order++;

        idVec.push_back(idList);
    } else{
        syntax_error();
    }
    t = peek();
    if(t.token_type == COMMA){
        t = lexer.GetToken();
        parse_id_list();
    }
}
/*
//parameter list
//POLY F1 = X^2 + 2;
struct id_list* Parser::parse_id_list() {
    struct id_list* head = new id_list;
    struct id_list* newId ;

    //t = lexer.GetToken();
    if (t.token_type == ID){

        struct id_list* temp = head;
        while(temp != nullptr){
            if(temp->name != t.lexeme){
                temp = temp->next;
            } else{
                break;
            }
        }

        if (temp == nullptr){
            head->name = t.lexeme;
            head->order = order++;
            head->next = nullptr;
        }

    } else{
        syntax_error();
    }

    Token temp = peek();
    if(temp.token_type == COMMA){
        newId = parse_id_list();
        head->next = newId;
    }
    lexer.UngetToken(t);
    t = lexer.GetToken();
    return head;
}
*/


//F,G,K
string Parser::parse_polynomial_name() {
    t = expect(ID);
    //lexer.UngetToken(t);
    return t.lexeme;
}

//monomials with operator same as body
struct term_list* Parser::parse_term_list() {
    struct term_list* termList = new term_list;
    struct term_list* newTerm;
    termList->term = parse_term();
    t = peek();
    if(t.token_type == PLUS || t.token_type == MINUS) {
        termList->addOperator = parse_add_operator();
        newTerm = parse_term_list();
        termList->next = newTerm;
    }
    return termList;
}

//single term like x^2y^2
struct term_struct* Parser::parse_term() {
    struct term_struct* termInfo = new term_struct;
    t = peek(); //ID POWER NUM
    //3x, 3 + , 3;
    if(t.token_type == NUM){
        termInfo->coefficient = parse_coefficient();
        //t = lexer.GetToken();
        t = peek();
        if(t.token_type == ID) {
            termInfo->mono_list = parse_monomial_list();
        } //else{
        //lexer.UngetToken(t);
        //}

    }else if(t.token_type == ID){
        termInfo->coefficient = 1;
        termInfo->mono_list = parse_monomial_list();

    }else{
        syntax_error();
    }
    monoVec.clear();
    return termInfo;
}

//monomial list
vector<monomial*> Parser::parse_monomial_list() {
    struct monomial* newMono;
    newMono = parse_monomial();
    monoVec.push_back(newMono);
    t = peek();
    if(t.token_type == ID) {
        parse_monomial_list();
    }
    return monoVec;
}

//x^2 -> (0,2)
struct monomial* Parser::parse_monomial() {
    struct monomial* monoInfo = new monomial;
    t = lexer.GetToken();
    if(t.token_type == ID){
        //id_list* paramList = parse_id_list();
        //lexer.UngetToken(t);
        /*
        while( paramList != nullptr){
            if(t.lexeme == paramList->name){
                monoInfo->order = paramList->order;
            }
            paramList = paramList->next;
        }
        */
        for(size_t i = 0; i < idVec.size(); i++){
            if(idVec[i]->name == t.lexeme){
                monoInfo->order = idVec[i]->order;
            }
        }
        //t = lexer.GetToken();
        monoInfo->exponent = parse_exponent();
    }
    return monoInfo;
}


int Parser::parse_exponent() {
    t = lexer.GetToken();
    if(t.token_type == POWER){
        t = expect(NUM);
        return stoi(t.lexeme);
    }else{
        lexer.UngetToken(t);
    }

    return 1;
}

//poly body operator
TokenType Parser::parse_add_operator() {
    t = lexer.GetToken();
    if(t.token_type != PLUS && t.token_type != MINUS) {
        syntax_error();
    }
    return t.token_type;
}

int Parser::parse_coefficient() {
    t = lexer.GetToken();
    if(t.token_type != NUM) {
        syntax_error();
    }
    return stoi(t.lexeme);
}

//START command
struct stmt* Parser::parse_start() {
    struct stmt* st;
    t = lexer.GetToken();
    if(t.token_type == START){
        st = parse_statement_list();
    }
    return st;
}

//input number vector
size_t tracer = 1;
size_t counter = 0;
void Parser::parse_inputs(stmt* st) {
    t = expect(NUM);
    string debug = t.lexeme;
    inputNum.push_back(stoi(t.lexeme));

    t = peek();
    if(t.token_type == NUM) {
        tracer++;
        if (tracer % polyVec.size() != 0){
            parse_inputs(st);
        } else{
            execute_program(st);
        }

    } else if (t.token_type == NUM){
        execute_program(st);

    }
}
/*
void Parser::parse_statement_list() {
    t = peek();
    if(t.token_type == INPUT || t.token_type == ID){
        stmt* st = parse_statement();
        stmtVec.push_back(st);
    }else{
        syntax_error();
    }
    t = peek();
    if (t.token_type == INPUT || t.token_type == ID){
        parse_statement_list();
    }
}
*/

struct stmt* Parser::parse_statement_list() {
    stmt* st;
    t = peek();
    if(t.token_type == INPUT || t.token_type == ID){
        st = parse_statement();
    }else{
        syntax_error();
    }
    t = peek();
    if (t.token_type == INPUT || t.token_type == ID){
        st->next = parse_statement_list();
    }
    return st;
}

struct stmt* Parser::parse_statement() {
    struct stmt* st;
    t = peek();
    if(t.token_type == INPUT){
        st = parse_input_statement();
    }
    else{
        st = parse_poly_evaluation_statement();
    }
    return st;
}

//when input is INPUT X
struct stmt* Parser::parse_input_statement() {
    struct stmt* st = new stmt;
    expect(INPUT);
    t = expect(ID);
    st->stmt_type = INPUT;
    if(inputMap.count(t.lexeme) == 0){
        inputMap[t.lexeme] = varIdx++;
        st->variable = INT_MAX;
        st->param_idx = inputMap[t.lexeme];

    } else{
        st->variable = INT_MAX;
        st->param_idx = inputMap[t.lexeme];
    }

    expect(SEMICOLON);
    return st;
}

//when input is F(X)
struct stmt* Parser::parse_poly_evaluation_statement() {
    struct stmt* st = new stmt;
    st->stmt_type = POLYEVAL;
    st->poly = parse_polynomial_evaluation();
    expect(SEMICOLON);
    return st;
}


struct poly_eval* Parser::parse_polynomial_evaluation() {
    struct poly_eval* polyEval = new poly_eval;
    t = peek();
    polyEval->polyName = parse_polynomial_name();
    argVec.clear();
    t = expect(LPAREN);
    parse_argument_list();
    polyEval->args = argVec;
    expect(RPAREN);
    return polyEval;
}

void Parser::parse_argument_list() {
    argVec.push_back(parse_argument());
    t = lexer.GetToken();
    if(t.token_type == COMMA){
        parse_argument_list();
    }else{
        lexer.UngetToken(t);
    }
}

struct argument* Parser::parse_argument() {
    struct argument* arg = new argument;
    t = lexer.GetToken();
    arg->polyEval = nullptr;
    if(t.token_type == ID){
        for(auto & i : polyVec){
            if (i->header->name == t.lexeme){ //F(F(F())); F(X);
                arg->arg_type = POLYEVAL;
                arg->polyEval = parse_polynomial_evaluation();
                break;
            }
        }

        if (arg->polyEval == nullptr){
            arg->arg_type = ID;
            arg->var_idx = inputMap[t.lexeme];
        }

    }
    else if(t.token_type == NUM){
        arg->arg_type = NUM;
        arg->const_val = stoi(t.lexeme);
    } else{
        syntax_error();
    }
    return arg;
}

vector<int> evalTerm;
void Parser::execute_program(struct stmt * start){
    struct stmt* ptr;
    int result;
    //assign_num();
    ptr = start;
    try{
        while(ptr != nullptr){
            switch (ptr->stmt_type) {
                case POLYEVAL:
                    result = 0;
                    evaluate_polynomial(ptr->poly);
                    if (evalTerm.empty()){
                        cout << "something went huang." << endl;
                    }

                    for (auto &i : evalTerm){
                        result += i;
                    }

                    cout << result << endl;
                    //counter = 0;
                    evalTerm.clear();
                    break;

                case INPUT:
                    if (counter >= inputNum.size()){
                        parse_inputs(ptr);
                    }

                    memory[ptr->param_idx] = inputNum[counter];
                    counter++;
                    break;
            }

            ptr = ptr->next;
        }
    } catch (exception &){

    }

    exit(1);
}


int evaluate_polynomial(struct poly_eval* poly) {
    vector<int> val;
    //vector<int> coefficient;
    vector<int> exponents;
    argVec = poly->args;
    int res = 1;
    for (size_t i = 0; i < polyVec.size(); i++) {
        if (polyVec[i]->header->name == poly->polyName) {
            for (size_t j = 0; j < argVec.size(); j++) {
                if (argVec[j]->const_val == INT_MAX) {
                    if (argVec[j]->polyEval == nullptr) {
                        //cout << "DBUG: " << memory[argVec[j]->var_idx] << endl;
                        val.push_back(memory[argVec[j]->var_idx]);
                    } else {
                        val.push_back(evaluate_polynomial(argVec[j]->polyEval));
                    }
                } else {
                    val.push_back(argVec[j]->const_val);
                }
            }

            term_list* terms = polyVec[i]->body->body;
            res = eval_term(terms, val);
            val.clear();
            break;
        }
    }

    return res;
}


bool firstEvaluation = true;
int eval_term(struct term_list* terms, vector<int> val) {

    int res = 1;
    //coefficient.push_back(polyVec[i]->body->body->term->coefficient);
    //2x^2+y^2
    int coefficient = terms->term->coefficient;
    monoVec = terms->term->mono_list;
    if (!monoVec.empty()){
        for(size_t k = 0; k < monoVec.size(); k++){
            int exponent = terms->term->mono_list[k]->exponent;
            int orderDebug = monoVec[k]->order;
            int value = val[monoVec[k]->order];
            res *= pow(value, exponent);
        }
    }

    res *= coefficient;
    if (firstEvaluation){
        evalTerm.push_back(res);
    }

    firstEvaluation = false;
    switch(terms->addOperator){
        case PLUS:
            evalTerm.push_back(eval_term(terms->next, val));
            break;
        case MINUS:
            evalTerm.push_back(eval_term(terms->next, val) * -1);
            break;
    }

    firstEvaluation = true;
    return res;
}


int main()
{
    Parser parser;
    parser.parse_input();
    return 0;
}
