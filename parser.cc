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
#include <iterator>
#include <sstream>
#include <map>
#include <cstddef>
#include <algorithm>

bool startProgram(std::map<std::string, int> map);

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
Token Parser::peek() {
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

int decryptVector(vector<std::string> v1, map<std::string, int> v2, vector<std::string> v3){
    int answer = 0, tempNum;
    Parser parser;
    string temp;
    int idx = 0;
    string pieces = "START ";

    for (int i = 0; i < v1.size() - 1; i += 2){
        if (v1[i + 1] == "PARAM"){
            pieces += std::to_string(v2[v3[idx]]) + " ";
            idx ++;

        } else{
            pieces += v1[i] + " ";
        }

    }

    stringstream ss;
    ss << pieces;

    while (!ss.eof()){
        ss >> temp;
        if (temp == "START"){
            ss >> temp;
            if (stringstream(temp) >> tempNum){
                answer += tempNum;
            }

        } else if (temp == "^"){
            ss >> temp;
            if (stringstream(temp) >> tempNum){
                answer = pow(answer, tempNum);
            }
        } else if (temp == "*"){
            ss >> temp;
            if (stringstream(temp) >> tempNum){
                answer *= tempNum;
            }


        } else if (temp == "+"){
            ss >> temp;
            if (stringstream(temp) >> tempNum){
                answer += tempNum;
            }
        } else if (temp == "-"){
            ss >> temp;
            if (stringstream(temp) >> tempNum){
                answer -= tempNum;
            }
        }
    }

    return answer;

}

std::string symbolToString(Token tok){
    switch (tok.token_type){
        case MINUS:
            return "-";

        case PLUS:
            return "+";

        case POWER:
            return "^";
    }

    return "";
}

bool startProgram(map<std::string, int> map) {
    std::map<std::string, int>::iterator it;
    for (it = map.begin(); it != map.end(); it++){
            if (it->second == INT_MAX){
            return false;
        }
    }

    return true;
}

int main() {
    LexicalAnalyzer lexer;
    Token t;
    Parser parser;

    struct polyStruct {
        std::string varName;
        int varCount = 0;
        std::vector<std::string> operation;
        std::vector<std::string> param;
        int line = -1;
    };

    struct analyze {
        std::vector<std::string> operation;
        int varCount = 0;
    };

    bool firstLoop;
    bool programStart = false;

    vector<std::string> tempVarList;
    polyStruct newPoly = polyStruct();
    analyze analyzer = analyze();
    std::vector<std::string> inOrder;
    std::vector<std::string> polyOrder;
    std::vector<struct polyStruct> polyList;
    std::vector<struct analyze> analyzeList;
    std::map<std::string, int> firstAdd;

    int idx = 0;

    while (t.token_type != END_OF_FILE) {
        t = lexer.GetToken();
        switch (t.token_type) {
            case POLY:
                if (programStart) {
                    parser.syntax_error();
                }

                firstLoop = true;

                //if it is the end of the POLY declaration.
                while (t.token_type != SEMICOLON) {
                    if (firstLoop) {
                        t = lexer.GetToken();
                        //check if there is ID.
                        if (t.token_type != ID) {
                            parser.syntax_error();
                        } else {
                            //check if ID is already declared.
                            if (!polyList.empty()) {
                                for (auto &i : polyList) {
                                    if (i.varName == t.lexeme) {
                                        if (parser.errorCode == 0) {
                                            parser.errorCode = 1;
                                        }
                                        if (!(find(parser.errorLine1.begin(), parser.errorLine1.end(), i.line) != parser.errorLine1.end()))
                                            parser.errorLine1.push_back(i.line);
                                        if (!(find(parser.errorLine1.begin(), parser.errorLine1.end(), t.line_no) != parser.errorLine1.end()))
                                            parser.errorLine1.push_back(t.line_no);

                                    } else{
                                        newPoly.varName = t.lexeme;
                                        newPoly.line = t.line_no;
                                    }
                                }
                            } else{
                                newPoly.varName = t.lexeme;
                                newPoly.line = t.line_no;
                            }
                        }

                        t = lexer.GetToken();

                        //check if the equal sign is followed by the variable.
                        if (t.token_type != EQUAL) {
                            if (t.token_type != LPAREN) {
                                parser.syntax_error();
                            }

                            while (t.token_type != RPAREN) {
                                t = lexer.GetToken();
                                if (t.token_type != ID) {
                                    parser.syntax_error();
                                }

                                if((find(tempVarList.begin(), tempVarList.end(), t.lexeme) == tempVarList.end())){
                                    tempVarList.push_back(t.lexeme);
                                }

                                t = lexer.GetToken();

                                if (t.token_type != COMMA && t.token_type != RPAREN) {
                                    parser.syntax_error();
                                }
                            }

                            t = lexer.GetToken();
                            if (t.token_type != EQUAL) {
                                parser.syntax_error();
                            }
                        }
                        t = lexer.GetToken();
                    }

                    //check coefficient.
                    if (t.token_type == NUM) {
                        newPoly.operation.push_back(t.lexeme);
                        newPoly.operation.emplace_back("NUM");
                    } else if (t.token_type == ID) {
                        std::string varName = t.lexeme;
                        if (!tempVarList.empty()) {
                            if ((std::find(tempVarList.begin(), tempVarList.end(), varName) == tempVarList.end())) {
                                if (parser.errorCode == 0 || parser.errorCode == 2) {
                                    parser.errorCode = 2;
                                    parser.errorLine2.push_back(t.line_no);
                                }
                            }
                        } else{
                            if (t.lexeme == "x"){
                                newPoly.operation.push_back(varName);
                                newPoly.operation.emplace_back("PARAM");
                                newPoly.varCount += 1;
                                newPoly.param.push_back(t.lexeme);
                                firstAdd[t.lexeme] = t.line_no;
                            } else{
                                parser.errorCode = 2;
                                parser.errorLine2.push_back(t.line_no);
                            }
                        }

                    } else if (t.token_type == POWER || t.token_type == PLUS || t.token_type == MINUS) {
                        std::string currentOperation = symbolToString(t);
                        t = lexer.GetToken();
                        if (t.token_type != NUM && t.token_type != ID) {
                            parser.syntax_error();
                        }

                        newPoly.operation.push_back(currentOperation);
                        newPoly.operation.emplace_back("OPERATION");
                        newPoly.operation.push_back(t.lexeme);

                        if (t.token_type == NUM){
                            newPoly.operation.emplace_back("NUM");

                        } else{
                            if ((find(newPoly.param.begin(), newPoly.param.end(), t.lexeme) == newPoly.param.end())){
                                newPoly.varCount += 1;
                                newPoly.param.push_back(t.lexeme);
                                newPoly.operation.emplace_back("PARAM");
                            }
                        }


                    }

                    t = lexer.GetToken();


                    if (t.token_type == SEMICOLON) {
                        polyList.push_back(newPoly);

                        newPoly.varCount = 0;
                        break;
                        
                    //Two operators times.
                    } else if (t.token_type == ID) {
                        newPoly.operation.emplace_back("*");
                        newPoly.operation.emplace_back("OPERATION");
                        newPoly.operation.push_back(t.lexeme);
                        if (tempVarList.empty()){
                            if (t.lexeme == "x"){
                                newPoly.param.push_back(t.lexeme);
                                newPoly.operation.emplace_back("PARAM");
                            }

                        } else {
                            if ((std::find(tempVarList.begin(), tempVarList.end(), t.lexeme) == tempVarList.end())) {
                                parser.errorCode = 2;
                                parser.errorLine2.push_back(t.line_no);
                            } else{
                                newPoly.param.push_back(t.lexeme);
                                newPoly.operation.emplace_back("PARAM");
                            }
                        }


                        //Doing operation
                    } else if (t.token_type == MINUS || t.token_type == PLUS || t.token_type == POWER) {
                        newPoly.operation.push_back(symbolToString(t));
                        newPoly.operation.emplace_back("OPERATION");
                    } else{
                        parser.syntax_error();
                    }

                    t = lexer.GetToken();
                    if (t.token_type != SEMICOLON) {
                        firstLoop = false;
                    } else {
                        polyList.push_back(newPoly);
                        break;
                    }
                }

                newPoly.varCount = 0;
                newPoly.operation.clear();
                newPoly.param.clear();
                tempVarList.clear();
                firstAdd.clear();

                //DEBUG CODE.
                /*for (int i = 0; i < polyList.size(); i++) {

                    cout << "Current stored variable: " << polyList[i].varName << endl;
                    cout << "param count: " << polyList[i].varCount << endl;
                    for (std::string inf : polyList[i].operation) {
                        cout << inf << " ";
                    }

                    cout << endl;
                }*/
                break;

            case START:
                programStart = true;
                break;

            case INPUT:
                if (!programStart) {
                    parser.syntax_error();
                }
                t = lexer.GetToken();
                if (t.token_type != ID) {
                    parser.syntax_error();
                }

                //Get input and make it in order.
                if (parser.inputOrder.size() == 0){
                    parser.inputOrder[t.lexeme] = INT_MAX;
                    inOrder.push_back(t.lexeme);
                } else{
                    if (parser.inputOrder.count(t.lexeme) == 0){
                        inOrder.push_back(t.lexeme);
                        parser.inputOrder[t.lexeme] = INT_MAX;
                    }
                }

                t = lexer.GetToken();

                if (t.token_type != SEMICOLON) {
                    parser.syntax_error();
                }

                break;

            default:
                //if no POLY is defined, print error.
                if (!programStart) {
                    parser.syntax_error();
                }

                vector<std::string> doCalculation;
                int varSize = -1;
                //G1(Z);
                if (t.token_type == ID && parser.errorCode == 0) {
                    for (auto & i : polyList) {
                        if (i.varName == t.lexeme) {
                            analyzer.operation = i.operation;;
                            varSize = i.varCount;
                            inOrder = i.param;
                            polyOrder.push_back(t.lexeme);
                            break;
                        }
                    }

                    if (varSize == -1) {
                        parser.errorCode = 3;
                        parser.errorLine3.push_back(t.line_no);
                    }

                    t = lexer.GetToken();
                }


                if (t.token_type == LPAREN) {
                    //Two Conditions here:
                    //One: G(F);
                    //Two: G(3);
                    //In case one, it should be ID
                    //Case two it should be NUM
                    while (t.token_type != RPAREN) {
                        t = lexer.GetToken();
                        if (t.token_type == ID) {
                            std::string varName = t.lexeme;
                            if (parser.inputOrder.count(t.lexeme) > 0){
                                analyzer.varCount += 1;

                            } else if (parser.errorCode == 0 || parser.errorCode == 3) {
                                parser.errorCode = 3;
                                parser.errorLine3.push_back(t.line_no);

                            } else {
                                break;
                            }

                        //case two.
                        } else if(t.token_type == NUM){
                            parser.inputOrder[inOrder[idx]] = stoi(t.lexeme);
                            analyzer.varCount += 1;
                            idx++;
                        }
                    }

                    if (analyzer.varCount != varSize && parser.errorCode == 0) {
                        parser.errorCode = 4;
                        parser.errorLine4.push_back(t.line_no);
                    }

                    t = lexer.GetToken();
                    if (t.token_type != SEMICOLON) {
                        parser.syntax_error();
                    }

                //The case that it is a series of numbers.
                } else if (t.token_type == NUM) {
                    for (int i = 0; i < polyOrder.size(); i++){
                        if (!parser.inputOrder.empty()){
                            struct polyStruct stmt = polyList[i];
                            int cap = stmt.varCount;
                            for (; idx < cap; idx ++){
                                parser.inputOrder[inOrder[idx]] = stoi(t.lexeme);
                                t = lexer.GetToken();
                            }
                        }
                    }

                }

                switch (parser.errorCode){
                    case 0:
                        if (analyzeList.empty()){
                            break;
                        }

                        if (startProgram(parser.inputOrder)){
                            for (auto &i : analyzeList){
                                int answer = decryptVector(i.operation, parser.inputOrder, inOrder);
                                cout << answer << " ";
                                parser.inputOrder.clear();
                            }

                            analyzeList.clear();
                            //clear analyzer to keep the structure clean for multiple runs.
                            analyzer.operation.clear();
                        }

                        break;

                    case 1:
                        cout << "Error Code 1: ";
                        sort(parser.errorLine1.begin(), parser.errorLine1.end());
                        for (int line : parser.errorLine1){
                            cout << line << " ";
                        }
                        exit(1);

                    case 2:
                        cout << "Error Code 2: ";
                        for (int line : parser.errorLine2){
                            cout << line << " ";
                        }
                        exit(1);

                    case 3:
                        cout << "Error Code 3: ";
                        for (int line : parser.errorLine3){
                            cout << line << " ";
                        }
                        exit(1);

                    case 4:
                        cout << "Error Code 4: ";
                        for (int line : parser.errorLine4){
                            cout << line << " ";
                        }
                        exit(1);

                }

                if (!analyzer.operation.empty()){
                    analyzeList.push_back(analyzer);
                }

                analyzer.operation.clear();
                analyzer.varCount = 0;
                break;
        }
    }

}



