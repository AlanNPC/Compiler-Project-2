/*
    Created by Jiang Xinrui
    filename : gradBuilder.h
    date : 2020.06.20
*/

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <set>

#include "IRMutator.h"


namespace Boost {

namespace Internal {

/*
   Used to gather all existing index names.
*/

class IndexMutator : public IRMutator{
   public:
      std::set<std::string> indexSet;
      Expr visit(Ref<const Index> op) override{
         if(indexSet.find(op->name) == indexSet.end()){
            indexSet.insert(op->name);
         }
         return Index::make(op->type(), op->name, op->dom, op->index_type);
      }
};

/*
   Used to gather all existing var names.
*/

class varMutator : public IRMutator{
   public:
      std::set<std::string> varSet;
      Expr visit(Ref<const Var> op) override{
         if(varSet.find(op->name) == varSet.end()){
            varSet.insert(op->name);
         }
         std::string varName = op->name + "<";
         for(int i = 0; i < op->shape.size(); i++){
            if(i > 0) varName += ", ";
            varName += std::to_string(op->shape[i]);
         }
         varName += ">";
         return Var::make(op->type(), varName, op->args, op->shape);
      }
};

/*
   Used to process index transformation according to rules(Map).
*/

class formChanger : public IRMutator{
   public:
      int curDom;
      std::map<std::string, Expr> rules;
      formChanger() : IRMutator() {
         curDom = 0;
      }
      Expr visit(Ref<const Var>) override;
      Expr visit(Ref<const Index>) override;
      Expr visit(Ref<const Binary>) override;
};

/*
   Core class.
   Used in back propagation.
*/

class gradBuilder : public IRMutator {
   public:
      Expr adjointExpr, accumExpr; //accumExpr deprecated
      Stmt adjointStmt;//statement we need to return
      std::string gradToVarName;//the name of the "grad var"
      
      /*
         global flags, used to control function behavior.
         general usage:
            flag = true;
            do_something();
            flag = false;

            void do_something(){
               if(flag){
                  ...
               }
               else{
                  ...
               }
            }
      */

      bool Gdebug;//debug option
      bool isLHS; //if it's a LHS
      bool isGradVar; //if it's the "grad var"
      bool isIndex; //if this Binary Expr is used as an index
      bool isInput; //if this var is currently one of the kernel.inputs
      bool isFirstStmt; //if adjointStmt is the first one(also the standard one)
      bool justGetName; //if we just want to get its name when visiting the node
      bool isCollect; //if we just want to collect information when visiting index
      bool isGetInt; //if we just want to get value when visiting Imm node
      
      
      std::string curIndexName; 
      int charCnt; //Used to generate new index name
      int curDom, curIndex;
      int DivModNum;
      formChanger changer;
      std::vector<std::string> IndexCollect;
      Expr curDst, curSrc; //Used when combining different adjointStmt
      Expr divIndex, modIndex; //Used in Binary index
      IndexMutator indexSetter; //Collect index name before core function
      gradBuilder() : IRMutator() {
         Gdebug = false;
         adjointExpr = Expr();
         isLHS = isGradVar = isIndex = isInput = false;
         isFirstStmt = true;
         isCollect = false;
         isGetInt = false;
         adjointStmt = Stmt();
         accumExpr = Expr();
         gradToVarName = "";
         charCnt = 0;
         justGetName = false;
         DivModNum = -1;
      }
      std::string genIndexName();
      Expr visit(Ref<const IntImm>) override;
      Expr visit(Ref<const UIntImm>) override;
      Expr visit(Ref<const FloatImm>) override;
      Expr visit(Ref<const StringImm>) override;
      Expr visit(Ref<const Unary>) override;
      Expr visit(Ref<const Binary>) override;
      Expr visit(Ref<const Select>) override;
      Expr visit(Ref<const Compare>) override;
      Expr visit(Ref<const Call>) override;
      Expr visit(Ref<const Var>) override;
      Expr visit(Ref<const Cast>) override;
      Expr visit(Ref<const Ramp>) override;
      Expr visit(Ref<const Index>) override;
      Expr visit(Ref<const Dom>) override;
      Stmt visit(Ref<const LoopNest>) override;
      Stmt visit(Ref<const IfThenElse>) override;
      Stmt visit(Ref<const Move>) override;
      Group visit(Ref<const Kernel>) override;
   private:
};


}  // namespace Internal

}  // namespace Boost