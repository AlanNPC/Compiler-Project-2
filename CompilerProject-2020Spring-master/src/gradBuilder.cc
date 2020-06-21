/*
    Created by Jiang Xinrui
    filename : gradBuilder.cc
    date : 2020.06.20
*/

#include "gradBuilder.h"
#include "IRPrinter.h"

namespace Boost {

namespace Internal{

Expr formChanger::visit(Ref<const Binary> op){
    Expr new_a = mutate(op->a);
    Expr new_b = mutate(op->b);
    return Binary::make(op->type(), op->op_type, new_a, new_b);
}

/*
    Before visit args(Index/Binary), we first update curDom to pass the Dom
*/

Expr formChanger::visit(Ref<const Var> op) {
    std::vector<Expr> new_args;
    for(int i = 0; i < op->args.size(); i++)
    {
        Expr arg = op->args[i];
        curDom = op->shape[i];
        new_args.push_back(mutate(arg));
    }
    return Var::make(op->type(), op->name, new_args, op->shape);
}

/*
    Change Index in adjointExpr according to the rules(Map : String -> Expr)
*/

Expr formChanger::visit(Ref<const Index> op) {
    if(rules.find(op->name) != rules.end()){
        Expr newIndex = rules[op->name];
        return newIndex;
    }
    Expr new_dom = mutate(op->dom);
    return Index::make(op->type(), op->name, new_dom, op->index_type);
}


/*
    When isGetInt=true, we pass op->value() to global DivModNum
    It's used to decide if divIndex and modIndex are bounded
*/

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

/*
    Index is a float var, and I didn't expect this
*/


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

/*
    It's used to generate a new index name. We start from "a", and then 
    search it in indexSetter.indexSet in case we generate an index which 
    already existed.
*/

std::string gradBuilder::genIndexName(){
    std::string str;
    do{
        char c = 'a' + (charCnt++);
        str = "";
        str += c;
    }while(indexSetter.indexSet.find(str) != indexSetter.indexSet.end());
    return str;
}

/*
    One of the core function.
    It deals with two cases:
    1. isIndex = false or isGradVar = false
        In this case, we see Binary Expr as a ternary expression c = a op b,
        and concerns about back propagation. For example c = a + b -> da = dc ;
        c = a * b -> db = a * dc. It's important to notice that for division 
        option, we only deal with cases like tensor = tensor / scalar, which 
        remains to be extended.

    2. isIndex = true and isGradVar = true
        In this case, Binary Expr appears at some index of the var , the gradient
        of which should be propagated. When this happens, we need to generate a
        new Index to replace Binary Expr and add (originName, reverse Expr) into
        changing rules for further use. For example, the var is B[p + q][r + s], 
        and its gradient needs to be propagated. Now we visit Binary Expr p + q,
        and check flags : isIndex and isGradVar. Both flags are set true. So we 
        generate a new index name 'a', and a new Index expr_a, so that we have
        a = p + q. We will always change the first var rather than the second, for
        the second one may be an immediate in many cases, but the first is always a
        var. The reverse Expr is a - q, so we add (p, Expr(a - q)) in change rules, 
        indicating all index p in adjointExpr should be changed into a - q. Thus we 
        gather the information used in index linear transformation.
        There's one more thing we need to pay attention to. That is case 8. Cases
        above shows one-to-one mapping, however, things changed when considering 
        division and mode options. For examples, B[i] = A[i + 2] -> dA[a] = dB[a - 2],
        but B[i] = A[i // 3] -> dA[a] = dB[a * 3] + dB[a * 3 + 1] + dB[a * 3 + 2].
        It's clearly a broadcast, which we are hard to support. Obviously, this has 
        been considered by teaching assistants, so case 8 : B[i] = A[i // 16][i % 16]
        is still one-to-one mapping. In this simplified case, we assume division and
        mode are bounded and will always come in pair. We set global divIndex and
        modIndex to handle this problem, and use DivModNum to check if these two indexes
        are bounded. Then we use them to restore origin index using composite Bianry
        Expr divIndex * DivModNum + modIndex. So we can deal with the following cases:
        a) B[i] = A[i // 16][i % 16] -> dA[j][k] = dB[j * 16 + k]
        b) B[i] = A[i % 16][i // 16] -> dA[j][k] = dB[k * 16 + j]
        c) B[i] = A[j][i % 16][k][m][i // 16] -> dA[j][a][k][m][b] = dB[b * 16 + a]
*/

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
        adjointExpr = tmp; // Forgetting to restore is disastrous
        return Binary::make(op->type(), op->op_type, new_a, new_b);
    }
    else{
        std::string newName = genIndexName();
        Type Itype = Type::int_scalar(32);
        Expr newIndex = Index::make(Itype, newName, Dom::make(Itype, 0, curDom), IndexType::Spatial);
        //We only deal with a op b, so var[a op b op c] will fail
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

/*
    One of the core function.
    It deals with the following cases:
    1. isLHS=true
        we only have one move in loopnest and one loopnest in kernel, so
        isLHS=true means this var should be transformed to gradient form and 
        accumulated in adjointExpr. Here we use reture value to change adjointExpr.
    2. isGradVar=true
        It means we reach a so-called "grad var"(which gradient should be propagated).
        Now we have adjointExpr as the right part of the assignment, var as the left
        part. We visit args of the var and update the changing rules. When finished, 
        we get the entire changing rules which should be used to change indexes in
        adjointExpr, so we call formChanger to do this. Then we will get an adjointExpr
        with no index trouble.
        If there's only one grad var in the statement, we simply set adjointStmt = 
        "var = adjointExpr" and return. However, in real world case, we always get a
        lot of grad vars, for examples A[i, j] = (B[i, j], B[i + 1, j])/2. So we should
        combine different adjointStmt together to form a new one. The problem is 
        different adjointStmt may have different index name, even if they represent
        the same meaning. For example, dB[i] = dA[i], dB[a] = dA[a]. To solve this
        we always consider the first adjointStmt as standard, and build mapping between
        indexes. For the case above, we add (a->i) into rules. After that, formChanger
        is called again to change indexes of the adjointStmt except the first one.
        When finished, we add the new adjointExpr back to the right part of the
        adjointStmt to form a new assignment.
    3. general var
        Just visit its args and return. When adjointExpr arrived at a general var
        (not a grad var), it's useless and just needs to look back upon.
*/


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

/*
    When justGetName = true or isCollect = true, it means we visit this
    node and only want to get some information. We use global variables
    to collect information.
*/

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
        if(Gdebug) std::cout << "In loop" << std::endl;
        new_body_list.push_back(mutate(body));
        if(Gdebug) std::cout << "Out loop " << std::endl;
    }
    if(Gdebug) std::cout << "body end" << std::endl;
    return LoopNest::make(new_index_list, new_body_list);
}


Stmt gradBuilder::visit(Ref<const IfThenElse> op) {
    Expr new_cond = mutate(op->cond);
    Stmt new_true_case = mutate(op->true_case);
    Stmt new_false_case = Stmt();
    return IfThenElse::make(new_cond, new_true_case, new_false_case);
}

/*
    We assume there's only one move in loopnest, so
    adjointExpr = newLHS is always a right choice.
*/

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

/*
    Before travelling, we need to gather all existing indexes into
    a set, so that we will never generate a new index having the
    same name as some existing one.
*/

Group gradBuilder::visit(Ref<const Kernel> op) {
    Group dontcare = indexSetter.mutate(op);
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