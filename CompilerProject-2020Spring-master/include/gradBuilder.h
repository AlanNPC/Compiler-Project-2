#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <set>

#include "IRMutator.h"
#include "IRVisitor.h"


namespace Boost {

namespace Internal {

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

class gradBuilder : public IRMutator {
   public:
      Expr adjointExpr, accumExpr;
      Stmt adjointStmt;
      std::string gradToVarName;
      bool isLHS;
      bool isGradVar;
      bool isIndex;
      bool isInput;
      bool isFirstStmt;
      int charCnt;
      bool justGetName;
      bool isCollect;
      bool isGetInt;
      std::string curIndexName;
      int curDom, curIndex;
      int DivModNum;
      formChanger changer;
      std::vector<std::string> IndexCollect;
      Expr curDst, curSrc;
      Expr divIndex, modIndex;
      gradBuilder() : IRMutator() {
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