#include "gradBuilder.h"
#include "IRPrinter.h"

namespace Boost {

namespace Internal{

bool Gdebug = false;

Expr formChanger::visit(Ref<const Binary> op){
    if(Gdebug) std::cout << "arrived!" << std::endl;
    Expr new_a = mutate(op->a);
    Expr new_b = mutate(op->b);
    return Binary::make(op->type(), op->op_type, new_a, new_b);
}

Expr formChanger::visit(Ref<const Var> op) {
    if(Gdebug) std::cout << "changer arrived at var " << op->name << std::endl;
    std::vector<Expr> new_args;
    for(int i = 0; i < op->args.size(); i++)
    {
        Expr arg = op->args[i];
        curDom = op->shape[i];
        new_args.push_back(mutate(arg));
    }
    if(Gdebug) std::cout << "changer leaved var " << op->name << std::endl;
    return Var::make(op->type(), op->name, new_args, op->shape);
}


Expr formChanger::visit(Ref<const Index> op) {
    if(Gdebug) std::cout << "changer arrived at index " << op->name << std::endl;
    if(rules.find(op->name) != rules.end()){
        Expr newIndex = rules[op->name];
        if(Gdebug) std::cout << "changer changed and leaved index " << op->name << std::endl;
        return newIndex;
    }
    Expr new_dom = mutate(op->dom);
    if(Gdebug) std::cout << "changer leaved index " << op->name << std::endl;
    return Index::make(op->type(), op->name, new_dom, op->index_type);
}


Expr gradBuilder::visit(Ref<const IntImm> op) {
    if(Gdebug) std::cout << "enter Int " << op->value() << std::endl;
    if(isGetInt){
        DivModNum = op->value();
    }
    return op;
}


Expr gradBuilder::visit(Ref<const UIntImm> op) {
    if(isGetInt){
        DivModNum = op->value();
    }
    return op;
}


Expr gradBuilder::visit(Ref<const FloatImm> op) {
    if(isGetInt){
        DivModNum = (int)(op->value());
    }
    return op;
}


Expr gradBuilder::visit(Ref<const StringImm> op) {
    return op;
}


Expr gradBuilder::visit(Ref<const Unary> op) {
    Expr tmp = adjointExpr;
    adjointExpr = Unary::make(op->type(), op->op_type, adjointExpr);
    Expr new_a = mutate(op->a);
    adjointExpr = tmp;
    return Unary::make(op->type(), op->op_type, new_a);
}


std::string gradBuilder::genIndexName(){
    char c = 'a' + (charCnt++);
    std::string emptyStr;
    return emptyStr + c;
}

Expr gradBuilder::visit(Ref<const Binary> op) {
    if(!isIndex || !isGradVar){
        Expr tmp = adjointExpr;
        if(op->op_type == BinaryOpType::Mul || op->op_type == BinaryOpType::Div){
            adjointExpr = Binary::make(op->type(), op->op_type, tmp, op->b);
        }
        Expr new_a = mutate(op->a);
        if(op->op_type == BinaryOpType::Mul || op->op_type == BinaryOpType::Div){
            adjointExpr = Binary::make(op->type(), op->op_type, op->a, tmp);
        }
        Expr new_b = mutate(op->b);
        adjointExpr = tmp;
        return Binary::make(op->type(), op->op_type, new_a, new_b);
    }
    else{
        std::string newName = genIndexName();
        Type Itype = Type::int_scalar(32);
        Expr newIndex = Index::make(Itype, newName, Dom::make(Itype, 0, curDom), IndexType::Spatial);
        if(op->op_type == BinaryOpType::Add || op->op_type == BinaryOpType::Sub){
            justGetName = true;
            Expr new_a = mutate(op->a);
            justGetName = false;
            BinaryOpType newTp;
            if(op->op_type == BinaryOpType::Add){
                newTp = BinaryOpType::Sub;
            }
            else{
                newTp = BinaryOpType::Add;
            }
            changer.rules[curIndexName] = Binary::make(op->type(), newTp, newIndex, op->b);
        }
        else if(op->op_type == BinaryOpType::Div || op->op_type == BinaryOpType::Mod){
            justGetName = true;
            Expr new_a = mutate(op->a);
            justGetName = false;
            int tmp = DivModNum;
            isGetInt = true;
            if(op->b->node_type() == IRNodeType::IntImm){
                //std::cout << "It's an integer" << std::endl;
            }
            else{
                //std::cout << "It's not an integer" << std::endl;
            }
            Expr new_b = mutate(op->b);
            isGetInt = false;
            if(Gdebug){
                std::cout << "DivModNum pre : " << tmp << " after " << DivModNum << std::endl;
            }
            if(op->op_type == BinaryOpType::Div){
                divIndex = newIndex;
            }
            else{
                modIndex = newIndex;
            }
            if(tmp == DivModNum){
                Expr constVar = IntImm::make(Itype, DivModNum);
                if(Gdebug) std::cout << "In ! index name is " << curIndexName << std::endl;
                changer.rules[curIndexName] = Binary::make(op->type(), BinaryOpType::Add,
                                                Binary::make(op->type(), BinaryOpType::Mul,
                                                    divIndex, constVar), modIndex);
                DivModNum = -1;
            }
        }
        return newIndex;
    }
}


Expr gradBuilder::visit(Ref<const Compare> op) {
    Expr new_a = mutate(op->a);
    Expr new_b = mutate(op->b);
    return Compare::make(op->type(), op->op_type, new_a, new_b);
}


Expr gradBuilder::visit(Ref<const Select> op) {
    Expr new_cond = mutate(op->cond);
    Expr new_true_value = mutate(op->true_value);
    Expr new_false_value = mutate(op->false_value);
    return Select::make(op->type(), new_cond, new_true_value, new_false_value);
}


Expr gradBuilder::visit(Ref<const Call> op) {
    std::vector<Expr> new_args;
    for (auto arg : op->args) {
        new_args.push_back(mutate(arg));
    }
    return Call::make(op->type(), new_args, op->func_name, op->call_type);

}


Expr gradBuilder::visit(Ref<const Cast> op) {
    Expr new_val = mutate(op->val);
    return Cast::make(op->type(), op->new_type, new_val);
}


Expr gradBuilder::visit(Ref<const Ramp> op) {
    Expr new_base = mutate(op->base);
    return Ramp::make(op->type(), new_base, op->stride, op->lanes);
}


Expr gradBuilder::visit(Ref<const Var> op) {
    if(Gdebug) std::cout << op->name << std::endl;
    if(isLHS){
        return Var::make(op->type(), "d" + op->name, op->args, op->shape);
    }
    if(op->name == gradToVarName and !isInput){
        isGradVar = true;
    }
    std::vector<Expr> new_args;
    isIndex = true;
    for(int i = 0; i < op->args.size(); i++)
    {
        Expr arg = op->args[i];
        curDom = op->shape[i];
        new_args.push_back(mutate(arg));
    }
    isIndex = false;
    if(op->name == gradToVarName and !isInput){
        if(Gdebug){
            std::cout << "When entering : ";
            IRPrinter nprinter;
            std::cout << nprinter.print(Var::make(op->type(), op->name, new_args, op->shape));
            std::cout << std::endl;
            std::cout << nprinter.print(adjointExpr) << std::endl;
        }
        isGradVar = false;
        if(Gdebug){
            for(auto item : changer.rules){
                std::cout << "rules : " << item.first << std::endl;
            }
        }
        if(Gdebug) std::cout << op->name << " start to reduce." << std::endl;
        adjointExpr = changer.mutate(adjointExpr);
        changer.rules.erase(changer.rules.begin(), changer.rules.end());
        if(Gdebug){
            IRPrinter printer;
            std::cout << printer.print(adjointExpr) << std::endl;
        }
        if(isFirstStmt){
            Expr dVar = Var::make(op->type(), "d" + op->name, new_args, op->shape);
            curDst = dVar;
            IndexCollect.clear();
            if(Gdebug) std::cout << "start to collect index." << std::endl;
            isCollect = true;
            for(int i = 0; i < new_args.size(); i++){
                curIndex = i;
                Expr arg = new_args[i];
                Expr dontcare = mutate(arg);
            }
            isCollect = false;
            curSrc = adjointExpr;
            if(Gdebug) std::cout << "run out of the firstStmt." << std::endl;
            isFirstStmt = false;
        }
        else{
            isCollect = true;
            for(int i = 0; i < new_args.size(); i++){
                curIndex = i;
                Expr arg = new_args[i];
                Expr dontcare = mutate(arg);
            }
            isCollect = false;
            if(Gdebug) std::cout << op->name << " start to replace." << std::endl;
            adjointExpr = changer.mutate(adjointExpr);
            changer.rules.erase(changer.rules.begin(), changer.rules.end());
            curSrc = Binary::make(op->type(), BinaryOpType::Add, curSrc, adjointExpr);
        }
        adjointStmt = Move::make(curDst, curSrc, MoveType::MemToMem);
        IRPrinter printer;
        if(Gdebug) std::cout << printer.print(adjointStmt) << std::endl;
    }
    return Var::make(op->type(), op->name, new_args, op->shape);
}


Expr gradBuilder::visit(Ref<const Dom> op) {
    Expr new_begin = mutate(op->begin);
    Expr new_extent = mutate(op->extent);
    return Dom::make(op->type(), new_begin, new_extent);
}


Expr gradBuilder::visit(Ref<const Index> op) {
    if(justGetName){
        curIndexName = op->name;
        return Index::make(op->type(), op->name, op->dom, op->index_type);
    }
    if(isCollect){
        if(isFirstStmt){
            if(Gdebug) std::cout << "curIndex=" << curIndex << " " << op->name << std::endl;
            IndexCollect.push_back(op->name);
        }
        else{
            if(Gdebug) std::cout << "linked ! " << op->name << " -> " << IndexCollect[curIndex] << std::endl;
            changer.rules[op->name] = Index::make(op->type(), IndexCollect[curIndex],
                                                  op->dom, op->index_type);
        }
        return Index::make(op->type(), op->name, op->dom, op->index_type);
    }
    Expr new_dom = mutate(op->dom);
    return Index::make(op->type(), op->name, new_dom, op->index_type);
}


Stmt gradBuilder::visit(Ref<const LoopNest> op) {
    std::vector<Expr> new_index_list;
    std::vector<Stmt> new_body_list;
    for (auto index : op->index_list) {
        new_index_list.push_back(mutate(index));
    }
    for (auto body : op->body_list) {
        //std::cout << "In loop" << std::endl;
        new_body_list.push_back(mutate(body));
        //std::cout << "Out loop " << std::endl;
    }
    //std::cout << "body end" << std::endl;
    return LoopNest::make(new_index_list, new_body_list);
}


Stmt gradBuilder::visit(Ref<const IfThenElse> op) {
    Expr new_cond = mutate(op->cond);
    Stmt new_true_case = mutate(op->true_case);
    Stmt new_false_case = Stmt();
    return IfThenElse::make(new_cond, new_true_case, new_false_case);
}


Stmt gradBuilder::visit(Ref<const Move> op) {
    if(Gdebug){
        std::cout << "arrived move !" << std::endl;
        IRPrinter printer;
        std::cout << printer.print(op) << std::endl;
    }
    isLHS = true;
    Expr new_dst = mutate(op->dst);
    isLHS = false;
    adjointExpr = new_dst;
    Expr new_src = mutate(op->src);
    return Move::make(new_dst, new_src, op->move_type);
}

Group gradBuilder::visit(Ref<const Kernel> op) {
    std::vector<Expr> new_inputs;
    isInput = true;
    for (auto expr : op->inputs) {
        new_inputs.push_back(mutate(expr));
    }
    isInput = false;
    std::vector<Expr> new_outputs;
    for (auto expr : op->outputs) {
        new_outputs.push_back(mutate(expr));
    }
    std::vector<Stmt> new_stmt_list;
    for (auto stmt : op->stmt_list) {
        new_stmt_list.push_back(mutate(stmt));
    }
    return Kernel::make(op->name, new_inputs, new_outputs, new_stmt_list, op->kernel_type);
}


}

}