#include "IRBuilder.hpp"

IRBuilder::IRBuilder(IRModule* module) 
    : module(module), curr_function(nullptr), curr_block(nullptr), next_vreg(0), next_label(0), local_vars({}) {}

Operand* IRBuilder::newVirtualReg(Token data_type){
    return Operand::createVirtualReg(next_vreg++, data_type);
}
std::string IRBuilder::newLabel(){
    std::string label = "L" + std::to_string(next_label);
    next_label++;
    return label;    
}

Operand* IRBuilder::visitLiteralExpr(LiteralExpr* node){
    return Operand::createConstant(std::stoi(node->getValue()), node->getType());
}
Operand* IRBuilder::visitVariableExpr(VariableExpr* node){
    return local_vars[node->getName()];
}
Operand* IRBuilder::visitBinaryExpr(BinaryExpr* node){
    Operand* left = evaluateExpr(node->getLeft());
    Operand* right = evaluateExpr(node->getRight());
    Operand* dest = newVirtualReg(node->getType());

    IRInstruction* instr;
    switch(node->getOP()) {
        case Token::PLUS: instr = IRInstruction::createAdd(dest, left, right); break;
        case Token::MINUS: instr = IRInstruction::createSub(dest, left, right); break;
        case Token::STAR: instr = IRInstruction::createMul(dest, left, right); break;
        case Token::SLASH: instr = IRInstruction::createDiv(dest, left, right); break;
        case Token::PERCENT: instr = IRInstruction::createMod(dest, left, right); break;
        case Token::EQ: instr = IRInstruction::createEq(dest, left, right); break;
        case Token::NE: instr = IRInstruction::createNe(dest, left, right); break;
        case Token::LT: instr = IRInstruction::createLt(dest, left, right); break;
        case Token::GT: instr = IRInstruction::createGt(dest, left, right); break;
        case Token::LE: instr = IRInstruction::createLe(dest, left, right); break;
        case Token::GE: instr = IRInstruction::createGe(dest, left, right); break;
        case Token::BIT_AND: instr = IRInstruction::createAnd(dest, left, right); break;
        case Token::BIT_OR: instr = IRInstruction::createOr(dest, left, right); break;
        case Token::BIT_XOR: instr = IRInstruction::createXor(dest, left, right); break;
        case Token::SHL: instr = IRInstruction::createShl(dest, left, right); break;
        case Token::SHR: instr = IRInstruction::createShr(dest, left, right); break;
        default: instr = nullptr; break;
    }

    if(instr) { curr_block->addInstruction(instr); }
    return dest;
}
Operand* IRBuilder::visitUnaryExpr(UnaryExpr* node){
    Operand* operand = evaluateExpr(node->getOperand());
    Operand* dest = newVirtualReg(node->getType());

    IRInstruction* instr;
    switch(node->getOP()) { 
        case Token::MINUS: instr = IRInstruction::createNeg(dest, operand); break;
        case Token::NOT: instr = IRInstruction::createNot(dest, operand); break;
        case Token::BIT_NOT: instr = IRInstruction::createNot(dest, operand); break;
        default: instr = nullptr; break;
    }

    if(instr) { curr_block->addInstruction(instr); }
    return dest;
}
Operand* IRBuilder::visitCallExpr(CallExpr* node){
    std::string func_name = node->getFunctionName();

    std::vector<Operand*> call_args;
    for(const auto& args : node->getArguments()) {
        call_args.push_back(evaluateExpr(args));
    }

    Operand* dest = newVirtualReg(node->getType());
    IRInstruction* instr = IRInstruction::createCall(dest, func_name, call_args);
    curr_block->addInstruction(instr); 
    return dest;
}
Operand* IRBuilder::visitArrayAccessExpr(ArrayAccessExpr* node){
    Operand* array_addr = evaluateExpr(node->getArrayName());
    Operand* index = evaluateExpr(node->getIndex());

    int size = 0;
    switch(node->getType()) {
        case Token::S8: size = 1; break;
        case Token::S16: size = 2; break;
        case Token::S32: size = 4; break;
        case Token::US8: size = 1; break;
        case Token::US16: size = 2; break;
        case Token::US32: size = 4; break;
        case Token::BOOL: size = 1; break;
    }
    Operand* element_size = Operand::createConstant(size, node->getType());

    Operand* offset = newVirtualReg(node->getType());
    IRInstruction* mul_instr = IRInstruction::createMul(offset, index, element_size);
    curr_block->addInstruction(mul_instr);

    Operand* actual_addr = newVirtualReg(node->getType());
    IRInstruction* addr_instr = IRInstruction::createAdd(actual_addr, array_addr, offset);
    curr_block->addInstruction(addr_instr);

    Operand* dest = newVirtualReg(node->getType());
    IRInstruction* instr = IRInstruction::createLoad(dest, actual_addr);
    curr_block->addInstruction(instr);

    return dest;
}

void IRBuilder::visitVarDeclStmt(VarDeclStmt* node){
}
void IRBuilder::visitAssignStmt(AssignStmt* node){
}
void IRBuilder::visitIfStmt(IfStmt* node){
}
void IRBuilder::visitWhileStmt(WhileStmt* node){
}
void IRBuilder::visitForStmt(ForStmt* node){
}
void IRBuilder::visitReturnStmt(ReturnStmt* node){
}
void IRBuilder::visitSendStmt(SendStmt* node){
}
void IRBuilder::visitRecvStmt(RecvStmt* node){
}
void IRBuilder::visitBlockStmt(BlockStmt* node){
}
void IRBuilder::visitExprStmt(ExprStmt* node){
}

void IRBuilder::visitFunctionDecl(FunctionDecl* node){
}
void IRBuilder::visitProgram(Program* node){
}

Operand* IRBuilder::evaluateExpr(Expr* expr) {
    if(LiteralExpr* lit = dynamic_cast<LiteralExpr*>(expr)) {
        return visitLiteralExpr(lit);
    } else if(VariableExpr* var = dynamic_cast<VariableExpr*>(expr)) {
        return visitVariableExpr(var);
    } else if(BinaryExpr* bin = dynamic_cast<BinaryExpr*>(expr)) {
        return visitBinaryExpr(bin);
    } else if(UnaryExpr* una = dynamic_cast<UnaryExpr*>(expr)) {
        return visitUnaryExpr(una);
    } else if(CallExpr* call = dynamic_cast<CallExpr*>(expr)) {
        return visitCallExpr(call);
    } else if(ArrayAccessExpr* array = dynamic_cast<ArrayAccessExpr*>(expr)) {
        return visitArrayAccessExpr(array);
    } else {
        return nullptr;
    }
}