//
// Created by SRB on 2020/5/11.
//
#include <map>
#include <set>
#include <iostream>
#include <vector>

#include "IR.h"
#include "type.h"

using namespace std;
using namespace Boost::Internal;

class DParser {
private:
    Type index_type = Type::int_scalar(32);
    Type data_type;

    string leftExpr;
    vector <string> rightExpr;
    vector <string> rightOps;

    map<string, Expr> leftIndex; //"i"->i
    vector<Expr> leftIndexList;
    vector<size_t> leftShapeList;
    vector<Expr> leftIndexListNoRepeat;

    map<string, Expr> rightIndex;
    set<string> used;

    vector<Expr> restriction; // to build cond of IfThenElse
    vector<set<string>> restrictionIndexSet;
    vector<Expr> localRestriction;

    map<string,Expr> varMP;

    vector<Stmt> finalLoop;
public:
    Group kernel;

    DParser(){};

    void parse(string name,vector<std::string>& ins,string outs,string data_typeStr,string kernelStr);

    void clear();

    void processOne(string code);

    vector<string> split(string str,const vector<char> delim);

    bool includeMathOp(string expr);

    map<string,int> registerIndex(string str);

    Stmt build_loop(string expr);

    Stmt build_if_stmt(Stmt trueExpr);

    Expr build_cond(int ind);

    Expr parse_AddSub(string expr);

    Expr parse_Mod(string expr);

    Expr parse_MulDiv(string term);

    Expr parse_operand(string operand);

    vector<string> getIndexName(string str);

    void buildConstriction(string str);
};
