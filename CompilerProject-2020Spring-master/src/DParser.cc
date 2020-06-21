#include <cstring>
#include <vector>
#include <deque>
#include <iostream>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <set>

#include "IR.h"
#include "type.h"
#include "DParser.h"

using namespace std;
using namespace Boost::Internal;

static bool debug=false;

void DParser::parse(string name,vector<std::string>& ins,string outs,string data_typeStr,string kernelStr){
    string noEmpty="";
    for(int i=0,size=kernelStr.size();i<size;++i){
        if(kernelStr[i]==' ') continue;
        noEmpty=noEmpty+kernelStr[i];
    }
    vector<string> code=split(noEmpty,{';'});
    if(data_typeStr=="float")
        data_type=Type::float_scalar(32);
    else
        data_type=Type::int_scalar(32);
    for(int i=0,size=code.size();i<size;++i)
        processOne(code[i]);
    vector<Expr> in;
    for(int i=0,size=ins.size();i<size;++i){
        if(varMP.find(ins[i])!=varMP.end())
            in.push_back(varMP[ins[i]]);
        else{
            cout<<"ERROR:"<<ins[i]<<" not in varMP!"<<endl;
            exit(1);
        }
    }
    if(find(ins.begin(),ins.end(),outs)!=ins.end()) {
        kernel = Kernel::make(name, in, {}, finalLoop, KernelType::CPU);
    }
    else {
        if(varMP.find(outs)!=varMP.end())
            kernel = Kernel::make(name, in, {varMP[outs]}, finalLoop, KernelType::CPU);
        else{
            cout<<"ERROR:"<<outs<<" not in varMP!"<<endl;
            exit(1);
        }
    }
}

void DParser::clear(){
    if(debug) cout<<"In clear..."<<endl;
    rightExpr.clear();
    rightOps.clear();
    leftIndex.clear();
    leftIndexList.clear();
    leftIndexListNoRepeat.clear();
    leftShapeList.clear();
    rightIndex.clear();
    restrictionIndexSet.clear();
    restriction.clear();
    if(debug) cout<<"Out of clear..."<<endl;
}

vector<string> DParser::getIndexName(string str) {
    int record[] = {-1, -1, -1, -1};
    vector<string> indexName;
    for (int i = 0, size = str.size(); i < size; ++i) {
        if (str[i] == '<') record[0] = i;
        else if (str[i] == '>') record[1] = i;
        else if (str[i] == '[') record[2] = i;
        else if (str[i] == ']') {
            record[3] = i;
            vector<string> tmp = split(
                    str.substr(record[2] + 1, record[3] - record[2] - 1),
                    {','});
            for(auto ss:tmp){
                if(includeMathOp(ss))
                    continue;
                indexName.push_back(ss);
            }
        }
    }
    return indexName;
}

void DParser::processOne(string code){
    if(debug) cout<<"In processOne"<<endl;
    vector<string> tmp=split(code,{'='});
    if(tmp.size()<1) return;
    leftExpr=tmp[0];
    //rightExpr=split(tmp[1],{'+','-'});
    int bracket=0;
    int bracket1=0;
    int beg=0;
    if(tmp.size()>=2) {
        for (int i = 0, size = tmp[1].size(); i < size; ++i) {
            if (tmp[1][i] == '[') bracket++;
            else if (tmp[1][i] == ']') bracket--;
            else if (tmp[1][i] == '(') bracket1++;
            else if (tmp[1][i] == ')') bracket1--;
            else if ((tmp[1][i] == '+' || tmp[1][i] == '-') && bracket == 0 && bracket1 == 0) {
                rightExpr.push_back(tmp[1].substr(beg, i - beg));
                rightOps.push_back(tmp[1].substr(i, 1));
                beg = i + 1;
            }
        }
        if (beg != tmp[1].size()) {
            rightExpr.push_back(tmp[1].substr(beg));
        }
    }
    map<string,int> wholeIndexTmp=registerIndex(code);
    set<string> st;
    vector<string> leftIndexName=getIndexName(leftExpr);
    for(auto indexName:leftIndexName){
        if(wholeIndexTmp.find(indexName)==wholeIndexTmp.end()) {
            cout << "ERROR:" <<indexName<<",the index not found"<<endl;
            exit(1);
        }
        int scale=wholeIndexTmp[indexName];
        if(debug) cout<<"after registerIndex(leftExpr),left index is:"<<indexName<<endl;
        Expr dom_tmp = Dom::make(index_type, 0, scale);
        Expr tmp = Index::make(index_type, indexName, dom_tmp, IndexType::Spatial);
        leftIndex.insert(make_pair(indexName,tmp));
        leftIndexList.push_back(tmp);
        leftShapeList.push_back(scale);
        if(st.find(indexName)==st.end()){
            st.insert(indexName);
            leftIndexListNoRepeat.push_back(tmp);
        }
    }
    if(debug) cout<<"rightExpr Size:"<<rightExpr.size()<<endl;
    for(auto index:wholeIndexTmp){
        if(leftIndex.find(index.first)!=leftIndex.end())
            continue;
        Expr dom_tmp = Dom::make(index_type, 0, index.second);
        Expr tmp = Index::make(index_type, index.first, dom_tmp, IndexType::Spatial);
        rightIndex.insert(make_pair(index.first,tmp));
    }
    buildConstriction(code);
    vector<Stmt> main_stmt;

    Expr leftOperand=parse_operand(leftExpr);
    Expr rightOperand = parse_Mod(rightExpr[0]);
    for (int i = 1; i < rightExpr.size(); ++i)
    {
        if (rightOps[i - 1] == "+")
            rightOperand = Binary::make(data_type, BinaryOpType::Add, rightOperand, parse_Mod(rightExpr[i]));
        else
            rightOperand = Binary::make(data_type, BinaryOpType::Sub, rightOperand, parse_Mod(rightExpr[i]));
    }
    main_stmt.push_back(Move::make(leftOperand,rightOperand,MoveType::MemToMem));
    finalLoop.push_back(LoopNest::make(leftIndexListNoRepeat, {main_stmt}));
    clear();
    if(debug) cout<<"Out of processOne"<<endl;
}

void DParser::buildConstriction(string str){
    if(debug)
        cout<<"In buildConstriction"<<endl;
    int record[]={-1,-1,-1,-1};
    for(int i=0,size=str.size();i<size;++i) {
        if (str[i] == '<') record[0] = i;
        else if (str[i] == '>') record[1] = i;
        else if (str[i] == '[') record[2] = i;
        else if (str[i] == ']') {
            record[3] = i;
            vector<string> dom = split(str.substr(record[0] + 1, record[1] - record[0] - 1),{','});
            vector<string> indexName = split(str.substr(record[2] + 1, record[3] - record[2] - 1),{','});
            if (dom.size() != indexName.size()) {
                cout << "ERROR:In buildConstriction, str=" << str << endl;
                exit(1);
            } else {
                for (int i = 0,tmpSize = dom.size(); i < tmpSize; ++i) {
                    int scale = atoi(dom[i].c_str());
                    Expr expr=parse_AddSub(indexName[i]);
                    restriction.push_back(Compare::make(data_type,CompareOpType::LT,expr,Expr(scale)));
                    restrictionIndexSet.push_back(used);
                    used.clear();
                    if(debug) cout<<"end Loop:"<<i<<",restriction.size() is:"<<restriction.size()<<endl;
                }
            }
        }
    }
    if(debug)
        cout<<"Out of buildConstriction"<<endl;
}

vector<string> DParser::split(string str,const vector<char> delim){
    vector<string> ret;
    int size=str.size();
    int beg=0;
    for(int i=0;i<size;++i){
        if(str[i]==' ') continue;
        auto it=find(delim.begin(),delim.end(),str[i]);
        if(it!=delim.end()){
            if(i==beg) {
                beg++;
                continue;
            }
            string tmp=str.substr(beg, i - beg);
            int str_i=0;
            while(str_i<tmp.size()&&tmp[str_i]==' ')
                str_i++;
            ret.push_back(tmp.substr(str_i));
            beg = i + 1;
        }
    }
    if(beg!=size){
        string tmp=str.substr(beg);
        int str_i=0;
        while(str_i<tmp.size()&&tmp[str_i]==' ')
            str_i++;
        ret.push_back(tmp.substr(str_i));
    }
    if(debug){
        cout<<"In split..."<<endl;
        cout<<"---ret:"<<endl;
        for(auto str: ret){
            cout<<"------"<<str<<endl;
        }
        cout<<"Out of split..."<<endl;
    }
    return ret;
}

bool DParser::includeMathOp(string expr){
    char mathOp[]={'+','-','*','/','%'};
    int size=expr.size();
    for(int i=0;i<size;++i) {
        if (find(mathOp, mathOp + 5, expr[i])!=(mathOp+5)){
            return true;
        }
    }
    return false;
}

map<string,int> DParser::registerIndex(string str){
    if(debug)
        cout<<"In registerIndex..."<<endl;
    int size=str.size();
    vector<string> dom;
    vector<string> indexName;
    vector<pair<string,Expr>> ret;
    map<string,int> name2range;
    int record[]={-1,-1,-1,-1};
    for(int i=0;i<size;++i) {
        if (str[i] == '<') record[0] = i;
        else if (str[i] == '>') record[1] = i;
        else if (str[i] == '[') record[2] = i;
        else if (str[i] == ']') {
            record[3] = i;
            dom = split(str.substr(record[0] + 1, record[1] - record[0] - 1),{','});
            indexName = split(str.substr(record[2] + 1, record[3] - record[2] - 1),{','});
            if (dom.size() != indexName.size()) {
                cout << "ERROR:In registerIndex, str=" << str << endl;
                exit(1);
            } else {
                for (int i = 0,tmpSize = dom.size(); i < tmpSize; ++i) {
                    int scale = atoi(dom[i].c_str());
                    if (includeMathOp(indexName[i]))
                        continue;
                    if(debug){
                        cout<<"dom:"<<scale<<" indexName:"<<indexName[i]<<endl;
                    }
                    if(name2range.find(indexName[i])!=name2range.end()){
                        if(name2range[indexName[i]]>scale)
                            name2range[indexName[i]]=scale;
                    }
                    else
                        name2range[indexName[i]]=scale;
                }
            }
        }
    }
    if(debug)
        cout<<"Out registerIndex..."<<endl;
    return name2range;
}

Stmt DParser::build_loop(string expr){
    if(debug) cout<<"In build_loop..."<<endl;
    Expr parsed_expr=parse_Mod(expr);
    if(!parsed_expr.defined())
        return Stmt();
    Expr moveLeft= Var::make(data_type, "tmp",leftIndexList, leftShapeList);
    Expr moveRight= Binary::make(data_type,BinaryOpType::Add,moveLeft,parsed_expr);
    Stmt moveStmt=Move::make(moveLeft,moveRight,MoveType::MemToMem);
    Stmt if_stmt=build_if_stmt(moveStmt);
    vector<Expr> loopIndex;
    for(auto i:used){
        loopIndex.push_back(rightIndex[i]);
    }
    if(debug) cout<<"Out of build_loop..."<<endl;
    return LoopNest::make(loopIndex, {if_stmt});
}

static bool subseteq(set<string> & A,set<string> & B){
    for(auto str: A){
        if(B.find(str)==B.end())
            return false;
    }
    return true;
}

Stmt DParser::build_if_stmt(Stmt trueExpr){
    if(debug)
        cout<<"In build_if_stmt..."<<endl;
    Stmt empty = Stmt();
    for(int i=0,size=restriction.size();i<size;++i){
        if(debug) cout<<"loop:"<<i<<endl;
        set<string> index=restrictionIndexSet[i];
        if(debug){
            cout<<"The indexSet in restrictionIndexSet["<<i<<"],indexSet size:"<<index.size()<<endl;
            for(string str:index)
                cout<<str<<",";
        }
        if(debug)
            cout<<"Size of used:"<<used.size()<<endl;
        if(subseteq(index,used)) {
            localRestriction.push_back(restriction[i]);
            if(debug)
                cout<<"Size of localRestriction is:"<<localRestriction.size()<<endl;
        }
    }
    if(debug)
        cout<<"Size of localRestriction is:"<<localRestriction.size()<<endl;
    if(localRestriction.size()!=0) {
        Expr cond = build_cond(0);
        if(debug)
            cout<<"Out of build_if_stmt..."<<endl;
        return IfThenElse::make(cond, trueExpr, empty);
    } else {
        if(debug)
            cout<<"Out of build_if_stmt..."<<endl;
        return trueExpr;
    }
}

Expr DParser::build_cond(int ind){
    if(ind==localRestriction.size()-1)
        return localRestriction[ind];
    return Binary::make(data_type,BinaryOpType::And,localRestriction[ind],build_cond(ind+1));
}

Expr DParser::parse_AddSub(string expr) {
    if(debug) cout<<"In parse_AddSub...expr is:"<<expr<<endl;
    if(expr=="") {
        return Expr();
    }
    int bracket=0,bracket1=0;
    int beg=0,end=expr.size();
    while(expr[beg]==' '&&beg<end)
        ++beg;
    while(expr[end-1]==' '&&beg<end)
        --end;
    if(beg==end) {
        return Expr();
    }
    for(int i=end-1;i>=beg;--i){
        if(expr[i]=='(')
            bracket++;
        else if(expr[i]==')')
            bracket--;
        else if(expr[i]=='[')
            bracket1++;
        else if(expr[i]==']')
            bracket1--;
        else if((expr[i]=='+'||expr[i]=='-')&&bracket==0&&bracket1==0){
            Expr left= parse_AddSub(expr.substr(0, i));
            Expr right= parse_Mod(expr.substr(i + 1));
            if(expr[i]=='+')
                return Binary::make(data_type, BinaryOpType::Add, left, right);
            else
                return Binary::make(data_type, BinaryOpType::Sub, left, right);
        }
    }
    if(expr[beg]=='('&&expr[end-1]==')') {
        if(end-beg-2!=0)
            return parse_AddSub(expr.substr(beg + 1, end - beg - 2));
        else
            return Expr();
    }
    if(debug) cout<<"Out of parse_AddSub..."<<endl;
    return parse_Mod(expr);
}

Expr DParser::parse_Mod(string expr){
    if(debug) cout<<"In parse_Mod...expr is:"<<expr<<endl;
    if(expr=="") {
        return Expr();
    }
    int bracket=0,bracket1=0;
    int beg=0,end=expr.size();
    while(expr[beg]==' '&&beg<end)
        ++beg;
    while(expr[end-1]==' '&&beg<end)
        --end;
    if(beg==end) {
        return Expr();
    }
    for(int i=end-1;i>=beg;--i){
        if(expr[i]=='(')
            bracket++;
        else if(expr[i]==')')
            bracket--;
        else if(expr[i]=='[')
            bracket1++;
        else if(expr[i]==']')
            bracket1--;
        else if(expr[i]=='%'&&bracket==0&&bracket1==0){
            Expr left= parse_Mod(expr.substr(0, i));
            Expr right= parse_MulDiv(expr.substr(i + 1));
            return Binary::make(data_type, BinaryOpType::Mod, left, right);
        }
    }
    if(expr[beg]=='('&&expr[end-1]==')') {
        if(end-beg-2!=0)
            return parse_AddSub(expr.substr(beg + 1, end - beg - 2));
        else
            return Expr();
    }
    if(debug) cout<<"Out of parse_Mod..."<<endl;
    return parse_MulDiv(expr);
}

Expr DParser::parse_MulDiv(string term){
    if(debug) cout<<"In parse_MulDiv...term is:"<<term<<endl;
    if(term=="") {
        return Expr();
    }
    int bracket=0,bracket1=0;
    int beg=0,end=term.size();
    while(term[beg]==' '&&beg<end)
        ++beg;
    while(term[end-1]==' '&&beg<end)
        --end;
    if(beg==end) {
        return Expr();
    }
    for(int i=end-1;i>=beg;--i){
        if(term[i]=='(')
            bracket++;
        else if(term[i]==')')
            bracket--;
        else if(term[i]=='[')
            bracket1++;
        else if(term[i]==']')
            bracket1--;
        else if(term[i]=='*'&&bracket==0&&bracket1==0){
            Expr left=parse_MulDiv(term.substr(0,i));
            Expr right= parse_operand(term.substr(i + 1));
            return Binary::make(data_type, BinaryOpType::Mul, left, right);
        }
        else if(i-1>=beg&&term[i]=='/'&&term[i-1]=='/'&&bracket==0&&bracket1==0) {
            Expr left = parse_MulDiv(term.substr(0, i-1));
            Expr right = parse_operand(term.substr(i + 1));
            return Binary::make(data_type, BinaryOpType::Div, left, right);
        }
        else if(term[i]=='/'&&bracket==0&&bracket1==0){
            Expr left=parse_MulDiv(term.substr(0,i));
            Expr right= parse_operand(term.substr(i + 1));
            return Binary::make(data_type, BinaryOpType::Div, left, right);
        }
    }
    if(term[beg]=='('&&term[end-1]==')') {
        if(debug) cout<<"Out of parse_MulDiv..."<<endl;
        if(end-beg-2!=0)
            return parse_AddSub(term.substr(beg + 1, end - beg - 2));
        else
            return Expr();
    }
    if(debug) cout<<"Out of parse_MulDiv..."<<endl;
    return parse_operand(term);
}

Expr DParser::parse_operand(string operand){
    if(debug) cout<<"In parse_operand...operand is:"<<operand<<endl;
    if(operand=="") {
        if(debug) cout<<"Out of parse_operand..."<<endl;
        return Expr();
    }
    int beg=0,end=operand.size();
    while(operand[beg]==' '&&beg<end)
        ++beg;
    while(operand[end-1]==' '&&beg<end)
        --end;
    if(beg==end) {
        if(debug) cout<<"Out of parse_operand..."<<endl;
        return Expr();
    }
    if(operand[beg]=='('&&operand[end-1]==')') {
        if(debug) cout<<"Out of parse_operand..."<<endl;
        if(end-beg-2!=0)
            return parse_AddSub(operand.substr(beg + 1, end - beg - 2));
        else
            return Expr();
    }
    int record[]={-1,-1,-1,-1};
    for(int i=beg;i<end;++i){
        if(operand[i]=='<') record[0]=i;
        else if(operand[i]=='>') record[1]=i;
        else if(operand[i]=='[') record[2]=i;
        else if(operand[i]==']') record[3]=i;
    }

    if(record[0]==-1||record[1]==-1){//const
        if(operand[beg]>='0'&&operand[beg]<='9') { //number
            float value = atof(operand.substr(beg, end - beg).c_str());
            if(debug) cout<<"Out of parse_operand...In Expr(value)"<<endl;
            return Expr(value);
        }
        else{// index
            string name=operand.substr(beg,end-beg);
            if(leftIndex.find(name)!=leftIndex.end()) {
                if(debug) cout<<"Out of parse_operand...In leftIndex[name]"<<endl;
                return leftIndex[name];
            }
            if(rightIndex.find(name)!=rightIndex.end()) {
                used.insert(name);
                if(debug) cout<<"Out of parse_operand...In rightIndex[name]"<<endl;
                return rightIndex[name];
            }
            cout<<"ERROR: In parse_operand,"<<endl;
            cout<<"the index name:"<<name<<" not found!"<<endl;
            cout<<"the operand name:"<<operand<<endl;
            cout<<"record:"<<record[0]<<","<<record[1]<<","<<record[2]<<","<<record[3]<<endl;
            exit(1);
        }
    }
    else if(record[2]==-1||record[3]==-1){//scalar
        if(debug) cout<<"Out of parse_operand..."<<endl;
        string name=operand.substr(beg,record[0]-beg);
        Expr value=Var::make(data_type, name, {}, {});
        varMP.insert(make_pair(name,value));
        return value;
    }
    else {
        string name = operand.substr(beg, record[0] - beg);
        vector<string> tmpIndexExpr = split(operand.substr(record[2]+1, record[3] - record[2]-1),{','});
        vector<string> tmpShape = split(operand.substr(record[0]+1, record[1] - record[0]-1),{','});
        vector<size_t> shape;
        vector<Expr> indexExpr;
        for (int i = 0, size = tmpShape.size(); i < size; ++i) {
            if(debug) cout<<"Loop:"<<i<<",expr is:"<<tmpIndexExpr[i]<<endl;
            int value=atoi(tmpShape[i].c_str());
            shape.push_back(value);
            Expr expr=parse_AddSub(tmpIndexExpr[i]);
            indexExpr.push_back(expr);
        }
        if(debug) cout<<"step1"<<endl;
        Expr value=Var::make(data_type, name, indexExpr,shape);
        if(debug) cout<<"step2"<<endl;
        varMP.insert(make_pair(name,value));
        if(debug) cout<<"Out of parse_operand..."<<endl;
        return value;
    }
}