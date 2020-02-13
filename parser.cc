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
#include <algorithm>

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

struct error_struct{
    int errorCode = 0;
    vector<int> errorLine;
};

Token t;
int varIdx = 0;
map<string, int> inputMap;
int memory[100];
vector<int> inputNum;
vector<struct id_list*> idVec;
vector<struct poly_decl_struct*> polyVec;
vector<monomial*> monoVec;
struct error_struct error;


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

void Parser::parse_poly_decl_section(){
    poly_decl_struct* newPoly = parse_poly_decl();
    if (!polyVec.empty()){
        for (auto &i : polyVec){
            if (newPoly->header->name == i->header->name){
                error.errorCode = 1;
                if (find(error.errorLine.begin(), error.errorLine.end(), i->header->decl_line) == error.errorLine.end()){
                    error.errorLine.push_back(i->header->decl_line);
                }
                if (find(error.errorLine.begin(), error.errorLine.end(), newPoly->header->decl_line) == error.errorLine.end()) {
                    error.errorLine.push_back(newPoly->header->decl_line);
                }
            }
        }
    }
    polyVec.push_back(newPoly);
    t = peek();
    if(t.token_type == POLY){
        parse_poly_decl_section();
    } else if (t.token_type == START){
        return;
    } else{
        syntax_error();
    }
}

struct poly_decl_struct* Parser::parse_poly_decl(){
    auto* poly = new poly_decl_struct;
    t = lexer.GetToken();
    if (t.token_type == POLY){
        poly->header = parse_polynomial_header();

        expect(EQUAL);
        poly->body = parse_polynomial_body();
        expect(SEMICOLON);

    } else {
        syntax_error();
    }

    return poly;
}

struct polynomial_body* Parser::parse_polynomial_body(){
    auto* body = new polynomial_body;
    body->body = parse_term_list();
    return body;
}

struct polynomial_header* Parser::parse_polynomial_header(){
    auto* header = new polynomial_header;
    auto* param = new id_list;
    param->name = "x";
    param->order = 0;
    idVec.clear();

    header->name = parse_polynomial_name();
    header->decl_line = t.line_no;
    t = peek();

    if(t.token_type == LPAREN){
        free(param);
        order = 0;
        t = lexer.GetToken();
        parse_id_list();

        t = expect(RPAREN);

    } else if (t.token_type == EQUAL){
        idVec.push_back(param);
    } else if (t.token_type != EQUAL){
        syntax_error();
    }

    header->idList = idVec;
    return header;
}


void Parser::parse_id_list() {
    auto* idList = new id_list;
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

string Parser::parse_polynomial_name() {
    t = expect(ID);
    return t.lexeme;
}


struct term_list* Parser::parse_term_list() {
    auto* termList = new term_list;
    struct term_list* newTerm;
    termList->term = parse_term();
    t = peek();
    if(t.token_type == PLUS || t.token_type == MINUS) {
        termList->addOperator = parse_add_operator();
        newTerm = parse_term_list();
        termList->next = newTerm;
    } else if (t.token_type != SEMICOLON){
        syntax_error();
    }
    return termList;
}

struct term_struct* Parser::parse_term() {
    auto* termInfo = new term_struct;
    t = peek();
    if(t.token_type == NUM){
        termInfo->coefficient = parse_coefficient();
        t = peek();
        if(t.token_type == ID) {
            termInfo->mono_list = parse_monomial_list();
        }

    } else if(t.token_type == ID){
        termInfo->coefficient = 1;
        termInfo->mono_list = parse_monomial_list();

    }else{
        syntax_error();
    }
    monoVec.clear();
    return termInfo;
}

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

struct monomial* Parser::parse_monomial() {
    auto* monoInfo = new monomial;
    t = lexer.GetToken();
    monoInfo->order = INT_MIN;
    if(t.token_type == ID){
        for(auto & i : idVec){
            if(i->name == t.lexeme){
                monoInfo->order = i->order;
                break;
            }
        }

        if (monoInfo->order == INT_MIN){
            error.errorCode = 2;
            error.errorLine.push_back(t.line_no);
        }
        monoInfo->exponent = parse_exponent();

    } else{
        syntax_error();
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

//operator logic.
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

//START
struct stmt* Parser::parse_start() {
    struct stmt *st = nullptr;
    t = lexer.GetToken();
    if(t.token_type == START){
        st = parse_statement_list();
    } else{
        syntax_error();
    }
    return st;
}

size_t counter = 0;
void Parser::parse_inputs(stmt* st) {
    //t = expect(NUM);
    t = lexer.GetToken();
    if (t.token_type != NUM){
        syntax_error();
    }

    string debug = t.lexeme;
    inputNum.push_back(stoi(t.lexeme));

    t = peek();
    if(t.token_type == NUM) {
        parse_inputs(st);

    } else if (t.token_type == START){
        execute_program(st);
    }

    else if (t.token_type == END_OF_FILE){
        try{
            execute_program(st);
        } catch (exception &err){
            exit(1);
        }

    } else{
        syntax_error();
    }
}

struct stmt* Parser::parse_statement_list() {
    stmt *st = nullptr;
    t = peek();
    if(t.token_type == INPUT || t.token_type == ID){
        st = parse_statement();
    }else{
        syntax_error();
    }

    t = peek();
    if (t.token_type == INPUT || t.token_type == ID){
        st->next = parse_statement_list();
    } else if (t.token_type == NUM || t.token_type == END_OF_FILE){
        return st;
    } else{
        syntax_error();
    }

    return st;
}

struct stmt* Parser::parse_statement() {
    struct stmt *st = nullptr;
    t = peek();
    if(t.token_type == INPUT){
        st = parse_input_statement();
    } else if (t.token_type == ID){
        st = parse_poly_evaluation_statement();
    } else{
        syntax_error();
    }
    return st;
}

//when input is INPUT X
struct stmt* Parser::parse_input_statement() {
    auto* st = new stmt;
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

struct stmt* Parser::parse_poly_evaluation_statement() {
    auto* st = new stmt;
    st->stmt_type = POLYEVAL;
    st->poly = parse_polynomial_evaluation();
    expect(SEMICOLON);
    return st;
}

struct poly_eval* Parser::parse_polynomial_evaluation() {
    auto* polyEval = new poly_eval;
    vector<struct id_list*> idList;
    bool find = false;
    auto* temp = new poly_eval;

    t = expect(ID);
    int errorCode3Num = t.line_no;
    for (auto &i : polyVec){
        if (i->header->name == t.lexeme){
            polyEval->polyName = t.lexeme;
            polyEval->line_no = t.line_no;
            find = true;
            break;
        }
    }

    if (!find){
        t = expect(LPAREN);
        parse_argument_list();
        t = expect(RPAREN);

        error.errorCode = 3;
        error.errorLine.push_back(errorCode3Num);
    }

    if (find){
        t = expect(LPAREN);
        temp = parse_argument_list();
        polyEval->args = temp->args;

        std::string polyName = polyEval->polyName;
        int nameLine;
        vector<argument*> tempStorage;
        for (auto &i : polyVec){
            if (i->header->name == polyName){
                idList = i->header->idList;
                nameLine = polyEval->line_no;
            }
        }

        if (temp->args.size() != idList.size()){
            if (error.errorCode == 0 || error.errorCode == 4){
                error.errorCode = 4;
                error.errorLine.push_back(nameLine);
            }
        }

        t = expect(RPAREN);


    }

    return polyEval;
}

struct poly_eval* Parser::parse_argument_list() {
    auto* temp = new poly_eval;
    argument* arg;
    t = peek();
    if(t.token_type == ID || t.token_type == NUM) {
        arg = parse_argument();
        temp->args.push_back(arg);
        temp->line_no = t.line_no;

        t = lexer.GetToken();
        if (t.token_type == COMMA) {
            auto* temp2 = parse_argument_list();
            for(auto &i : temp2->args){
                temp->args.push_back(i);
            }

        } else{
            lexer.UngetToken(t);
        }

    }else{
        syntax_error();
    }

    return temp;
}

int idx_test = 0;
struct argument* Parser::parse_argument() {
    auto* arg = new argument;
    t = lexer.GetToken();
    arg->polyEval = nullptr;
    idx_test++;
    if(t.token_type == ID){
        Token temp = lexer.GetToken();
        lexer.UngetToken(temp);
        lexer.UngetToken(t);
        if (temp.token_type == LPAREN){
            arg->arg_type = POLYEVAL;
            arg->polyEval = parse_polynomial_evaluation();
            return arg;

        } else {
            arg->arg_type = ID;
            t = lexer.GetToken();
            if (inputMap.count(t.lexeme) == 1) {
                arg->var_idx = inputMap[t.lexeme];
                arg->varName = t.lexeme;

            } else {
                error.errorCode = 5;
                error.errorLine.push_back(t.line_no);
                arg->var_idx = INT_MAX;
                return arg;
            }
        }

    } else if(t.token_type == NUM){
        arg->arg_type = NUM;
        arg->const_val = stoi(t.lexeme);
        arg->var_idx = idx_test;

    } else{
        syntax_error();
    }

    return arg;
}

vector<int> evalTerm;
void Parser::execute_program(struct stmt * start){
    struct stmt* ptr;
    int result;
    ptr = start;

    try{
        if (error.errorCode != 0){
            printf("Error Code %d: ", error.errorCode);
            sort(error.errorLine.begin(), error.errorLine.end());
            for (int i : error.errorLine){
                printf("%d ", i);
            }

            exit(1);

        } else{
            while(ptr != nullptr){
                switch (ptr->stmt_type) {
                    case POLYEVAL:
                        try{
                            result = evaluate_polynomial(ptr->poly);
                        } catch (exception &Err){
                            exit(1);
                        }

                        cout << result << endl;
                        evalTerm.clear();
                        break;

                    case INPUT:
                        if (counter >= inputNum.size()){
                            parse_inputs(ptr);
                        }

                        memory[ptr->param_idx] = inputNum[counter];
                        counter++;
                        break;

                    default:
                        break;
                }

                ptr = ptr->next;
            }
        }

    } catch (exception &){

    }

    exit(1);
}

bool isPoly = true;
int recursive = 0;
int evaluate_polynomial(struct poly_eval* poly) {
    vector<int> val;
    //vector<int> coefficient;
    vector<int> exponents;
    vector<struct argument*> argVec;
    argVec = poly->args;
    int res = 0;
    for (auto & i : polyVec) {
        if (i->header->name == poly->polyName) {
            for (auto & j : argVec) {
                if (j->const_val == INT_MAX) {
                    if (j->polyEval == nullptr) {
                        //cout << "DBUG: " << memory[argVec[j]->var_idx] << endl;

                        if (inputMap.count(j->varName) == 1){
                            val.push_back(memory[inputMap[j->varName]]);
                        } else{
                            val.push_back(memory[j->var_idx]);
                        }

                        if (recursive > 0){
                            term_list* terms = i->body->body;
                            eval_term(terms, val);
                            int result = 0;
                            for (auto &i : evalTerm){
                                result += i;
                            }
                            //memory[j->var_idx] = result;
                            evalTerm.clear();
                            recursive--;
                        }

                    } else {
                        try{
                            recursive ++;
                            val.push_back(evaluate_polynomial(j->polyEval));
                        } catch (exception &err){
                            exit(1);
                        }

                    }
                } else {
                    int constant = j->const_val;
                    int currentIdxForConst = j->var_idx;
                    val.push_back(j->const_val);
                }
            }

            term_list* terms = i->body->body;
            try{
                eval_term(terms, val);
                for (auto &i : evalTerm){
                    res += i;
                }
                evalTerm.clear();
            } catch (exception &err){
                exit(1);
            }


            break;
        }
    }

    recursive = false;
    return res;
}

int eval_term(struct term_list* terms, vector<int> val) {

    int res = 1;
    int coefficient = terms->term->coefficient;
    monoVec = terms->term->mono_list;
    if (!monoVec.empty()){
        for(size_t k = 0; k < monoVec.size(); k++){
            int exponent = terms->term->mono_list[k]->exponent;
            int orderDebug = monoVec[k]->order;
            int value = val[orderDebug];
            res *= pow(value, exponent);
        }
    }

    res *= coefficient;
    if (isPoly){
        evalTerm.push_back(res);
    }

    isPoly = false;
    switch(terms->addOperator){
        case PLUS:
            try{
                evalTerm.push_back(eval_term(terms->next, val));
            } catch (exception &Err){
                exit(1);
            }

            break;
        case MINUS:
            try{
                evalTerm.push_back(eval_term(terms->next, val) * -1);
            } catch (exception &Err){
                exit(1);
            }
            break;
    }

    isPoly = true;
    return res;
}

int main()
{
    Parser parser;
    parser.parse_input();
    return 0;
}
