#include "IR.hpp"
#include <iostream>
#include <algorithm>

Operand::Operand(OperandType type, int vreg_num, int const_value, int stack_offset, std::string* label_name, std::string* global_name, Token data_type) 
    : type(type), vreg_num(vreg_num), const_value(const_value), stack_offset(stack_offset), label_name(label_name), global_name(global_name), data_type(data_type) {}

Operand* Operand::createVirtualReg(int vreg_num, Token data_type) {
    return new Operand(OperandType::VIRTUAL_REG, vreg_num, 0, 0, nullptr, nullptr, data_type);
}
Operand* Operand::createConstant(int value, Token data_type){ 
    return new Operand(OperandType::CONSTANT, 0 , value, 0, nullptr, nullptr, data_type);
}
Operand* Operand::createLabel(const std::string& label_name){ 
    return new Operand(OperandType::LABEL, 0, 0, 0, new std::string(label_name), nullptr, Token::INVALID);
}
Operand* Operand::createGlobal(const std::string& global_name, Token data_type){ 
    return new Operand(OperandType::GLOBAL, 0, 0, 0, nullptr, new std::string(global_name), data_type);
}
Operand* Operand::createStackSlot(int offset, Token data_type){ 
    return new Operand(OperandType::STACK_SLOT, 0, 0, offset, nullptr, nullptr, data_type);
}

bool Operand::isVirtualReg() const { return type == OperandType::VIRTUAL_REG; }
bool Operand::isConstant() const { return type == OperandType::CONSTANT; }
bool Operand::isLabel() const { return type == OperandType::LABEL; }
bool Operand::isGlobal() const { return type == OperandType::GLOBAL; }
bool Operand::isStackSlot() const { return type == OperandType::STACK_SLOT; }
OperandType Operand::getType() const { return type; }
int Operand::getVregNum() const { return vreg_num; }
int Operand::constValue() const { return const_value; }
int Operand::stackOffset() const { return stack_offset; }
std::string* Operand::labelName() const { return label_name; }
std::string* Operand::globalName() const { return global_name; }
Token Operand::getDataType() const { return data_type; }
std::string Operand::toString() const { 
    switch(type) {
        case OperandType::VIRTUAL_REG:
            return "v" + std::to_string(vreg_num) + ":" + tokenToString(data_type);

        case OperandType::CONSTANT:
            return std::to_string(const_value) + ":" + tokenToString(data_type);
        
        case OperandType::LABEL:
            return *label_name;
        
        case OperandType::GLOBAL:
            return *global_name + ":" + tokenToString(data_type);
        
        case OperandType::STACK_SLOT:
            return "[sp+" + std::to_string(stack_offset) + "]:" + tokenToString(data_type);
        
        default:
            return "UNKNOWN";
    }
}

IRInstruction::IRInstruction(IROpcode opcode, Operand* dest, Operand* src1, Operand* src2, std::string* func_name, std::vector<Operand*> args, std::vector<std::pair<Operand*, std::string>> phi_operands)
    : opcode(opcode), dest(dest), src1(src1), src2(src2), func_name(func_name), args(args), phi_operands(phi_operands) {}
IRInstruction* IRInstruction::createAdd(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::ADD, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createSub(Operand* dest, Operand* src1, Operand* src2) { 
    return new IRInstruction(IROpcode::SUB, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createMul(Operand* dest, Operand* src1, Operand* src2) { 
    return new IRInstruction(IROpcode::MUL, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createDiv(Operand* dest, Operand* src1, Operand* src2) { 
    return new IRInstruction(IROpcode::DIV, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createMod(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::MOD, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createAnd(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::AND, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createOr(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::OR, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createXor(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::XOR, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createShl(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::SHL, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createShr(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::SHR, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createSar(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::SAR, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createEq(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::EQ, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createNe(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::NE, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createLt(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::LT, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createLe(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::LE, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createGt(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::GT, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createGe(Operand* dest, Operand* src1, Operand* src2) {
    return new IRInstruction(IROpcode::GE, dest, src1, src2, nullptr, {}, {});
}
IRInstruction* IRInstruction::createNeg(Operand* dest, Operand* src1) {
    return new IRInstruction(IROpcode::NEG, dest, src1, nullptr, nullptr, {}, {});
}
IRInstruction* IRInstruction::createNot(Operand* dest, Operand* src1) {
    return new IRInstruction(IROpcode::NOT, dest, src1, nullptr, nullptr, {}, {});
}
IRInstruction* IRInstruction::createLoad(Operand* dest, Operand* address) {
    return new IRInstruction(IROpcode::LOAD, dest, address, nullptr, nullptr, {}, {});
}
IRInstruction* IRInstruction::createStore(Operand* address, Operand* value) {
    return new IRInstruction(IROpcode::STORE, nullptr, address, value, nullptr, {}, {});
}
IRInstruction* IRInstruction::createAlloca(Operand* dest, Operand* size) {
    return new IRInstruction(IROpcode::ALLOCA, dest, size, nullptr, nullptr, {}, {});
}
IRInstruction* IRInstruction::createLabel(Operand* label) {
    return new IRInstruction(IROpcode::LABEL, label, nullptr, nullptr, nullptr, {}, {});
}
IRInstruction* IRInstruction::createJump(Operand* target) {
    return new IRInstruction(IROpcode::JUMP, nullptr, target, nullptr, nullptr, {}, {});
}
IRInstruction* IRInstruction::createReturn(Operand* value) {
    return new IRInstruction(IROpcode::RETURN, nullptr, value, nullptr, nullptr, {}, {});
}
IRInstruction* IRInstruction::createCopy(Operand* dest, Operand* src1) {
    return new IRInstruction(IROpcode::COPY, dest, src1, nullptr, nullptr, {}, {});
}
IRInstruction* IRInstruction::createSend(Operand* value, const std::string& target_func) {
    return new IRInstruction(IROpcode::SEND, nullptr, value, nullptr, new std::string(target_func), {}, {});
}
IRInstruction* IRInstruction::createRecv(Operand* dest) {
    return new IRInstruction(IROpcode::RECV, dest, nullptr, nullptr, nullptr, {}, {});
}
IRInstruction* IRInstruction::createOut(Operand* value) {
    return new IRInstruction(IROpcode::OUT, nullptr, value, nullptr, nullptr, {}, {});
}
IRInstruction* IRInstruction::createIn(Operand* dest) {
    return new IRInstruction(IROpcode::IN, dest, nullptr, nullptr, nullptr, {}, {});
}
IRInstruction* IRInstruction::createCall(Operand* dest, const std::string& func_name, const std::vector<Operand*>& args) {
    return new IRInstruction(IROpcode::CALL, dest, nullptr, nullptr, new std::string(func_name), args, {});
}
IRInstruction* IRInstruction::createPhi(Operand* dest, const std::vector<std::pair<Operand*, std::string>>& phi_operands) {
    return new IRInstruction(IROpcode::PHI, dest, nullptr, nullptr, nullptr, {}, phi_operands);
}
IRInstruction* IRInstruction::createBranch(Operand* condition, Operand* true_label) {
    return new IRInstruction(IROpcode::BRANCH, nullptr, condition, true_label, nullptr, {}, {});
}

IROpcode IRInstruction::getOpcode() const { return opcode; }
Operand* IRInstruction::getDest() const { return dest; }
Operand* IRInstruction::getSrc1() const { return src1; }
Operand* IRInstruction::getSrc2() const { return src2; }
std::string* IRInstruction::getFuncName() const { return func_name; }
std::vector<Operand*> IRInstruction::getArgs() const { return args; }
std::vector<std::pair<Operand*, std::string>> IRInstruction::getPhi() const { return phi_operands; }
static std::string opcodeToString(IROpcode op) {
    switch(op) {
        case IROpcode::ADD: return "ADD";
        case IROpcode::SUB: return "SUB";
        case IROpcode::MUL: return "MUL";
        case IROpcode::DIV: return "DIV";
        case IROpcode::MOD: return "MOD";
        case IROpcode::AND: return "AND";
        case IROpcode::OR: return "OR";
        case IROpcode::XOR: return "XOR";
        case IROpcode::SHL: return "SHL";
        case IROpcode::SHR: return "SHR";
        case IROpcode::SAR: return "SAR";
        case IROpcode::EQ: return "EQ";
        case IROpcode::NE: return "NE";
        case IROpcode::LT: return "LT";
        case IROpcode::LE: return "LE";
        case IROpcode::GT: return "GT";
        case IROpcode::GE: return "GE";
        case IROpcode::NEG: return "NEG";
        case IROpcode::NOT: return "NOT";
        case IROpcode::COPY: return "COPY";
        default: return "UNKNOWN";
    }
}
std::string IRInstruction::toString() const {
    std::string result = "";
    
    switch(opcode) {
        case IROpcode::ADD:
        case IROpcode::SUB:
        case IROpcode::MUL:
        case IROpcode::DIV:
        case IROpcode::MOD:
        case IROpcode::AND:
        case IROpcode::OR:
        case IROpcode::XOR:
        case IROpcode::SHL:
        case IROpcode::SHR:
        case IROpcode::SAR:
        case IROpcode::EQ:
        case IROpcode::NE:
        case IROpcode::LT:
        case IROpcode::LE:
        case IROpcode::GT:
        case IROpcode::GE:
            result = dest->toString() + " = ";
            result += opcodeToString(opcode) + " ";
            result += src1->toString() + ", " + src2->toString();
            break;
        case IROpcode::NEG:
        case IROpcode::NOT:
        case IROpcode::COPY:
            result = dest->toString() + " = ";
            result += opcodeToString(opcode) + " ";
            result += src1->toString();
            break;
        case IROpcode::LOAD:
            result = dest->toString() + " = LOAD " + src1->toString();
            break;
        case IROpcode::STORE:
            result = "STORE " + src1->toString() + ", " + src2->toString();
            break;
        case IROpcode::ALLOCA:
            result = dest->toString() + " = ALLOCA " + src1->toString();
            break;
        case IROpcode::LABEL:
            result = dest->toString() + ":";
            break;
        case IROpcode::JUMP:
            result = "JUMP " + src1->toString();
            break;
        case IROpcode::BRANCH:
            result = "BRANCH " + src1->toString() + ", " + src2->toString();
            break;
        case IROpcode::CALL:
            if (dest) result = dest->toString() + " = ";
            result += "CALL " + *func_name + "(";
            for (size_t i = 0; i < args.size(); i++) {
                result += args[i]->toString();
                if (i < args.size() - 1) result += ", ";
            }
            result += ")";
            break;
        case IROpcode::RETURN:
            result = "RETURN";
            if (src1) result += " " + src1->toString();
            break;
        case IROpcode::PHI:
            result = dest->toString() + " = PHI(";
            for (size_t i = 0; i < phi_operands.size(); i++) {
                if(phi_operands[i].first) {
                    result += phi_operands[i].first->toString();
                } else {
                    result += "[NULL]";
                }
                result += ", " + phi_operands[i].second;
                if (i < phi_operands.size() - 1) result += "), (";
            }
            result += ")";
            break;
        case IROpcode::SEND:
            result = "SEND ";
            if(src1) result += src1->toString();
            else result += "[NULL]";
            result += " TO ";
            if(func_name) result += *func_name;
            else result += "[NULL]";
            break;
        case IROpcode::RECV:
            result = dest->toString() + " = RECV";
            break;
        case IROpcode::OUT:
            result = "OUT " + src1->toString();
            break;
        case IROpcode::IN:
            result = dest->toString() + " = IN";
            break;
        default:
            result = "UNKNOWN";
    }
    return result;
}

BasicBlock::BasicBlock(const std::string& label) : label(label), instructions({}), successors({}), predecessors({}) {}
void BasicBlock::addInstruction(IRInstruction* insert) { instructions.push_back(insert); }
void BasicBlock::addSuccessor(BasicBlock* block) { successors.push_back(block); }
void BasicBlock::addPredecessor(BasicBlock* block) { predecessors.push_back(block); }
std::string BasicBlock::getLabel() const { return label; }
std::vector<IRInstruction*> BasicBlock::getInstructions() const { return instructions; }
std::vector<BasicBlock*> BasicBlock::getSuccessors() const { return successors; }
std::vector<BasicBlock*> BasicBlock::getPredecessors() const { return predecessors; }
void BasicBlock::insertInstructionBefore(int index, IRInstruction* instr) { instructions.insert(instructions.begin() + index, instr); }
void BasicBlock::removeInstruction(IRInstruction* instr) { instructions.erase(std::remove(instructions.begin(), instructions.end(), instr), instructions.end()); }
std::vector<IRInstruction*>& BasicBlock::getInstructionsMutable() { return instructions; }
std::string BasicBlock::toString() const {
    std::string result = label + ":\n";
    for (IRInstruction* instr : instructions) {
        if(instr == nullptr) {
            result += "    [NULL INSTRUCTION]\n";
        } else {
            result += "    " + instr->toString() + "\n";
        }
    }
    return result;
}

IRFunction::IRFunction(const std::string& name, const std::vector<Operand*> parameters, Token return_type) 
    : name(name), parameters(parameters), return_type(return_type), basic_blocks({}), entry_block(nullptr) {}
void IRFunction::addBasicBlock(BasicBlock* block) { basic_blocks.push_back(block); }
void IRFunction::setEntryBlock(BasicBlock* entry) { entry_block = entry; }
std::string IRFunction::getName() const { return name; }
std::vector<Operand*> IRFunction::getParameters() const { return parameters; }
Token IRFunction::getReturnType() const { return return_type; }
std::vector<BasicBlock*> IRFunction::getBasicBlocks() const { return basic_blocks; }
BasicBlock* IRFunction::getEntryBlock() const { return entry_block; }
std::string IRFunction::toString() const {
    std::string result = "FUNCTION " + name + "(";
    for (size_t i = 0; i < parameters.size(); i++) {
        result += parameters[i]->toString();
        if (i < parameters.size() - 1) result += ", ";
    }
    result += ") -> " + tokenToString(return_type) + "\n";
    for (BasicBlock* bb : basic_blocks) {
        result += bb->toString();
    }
    return result;
}

IRModule::IRModule() : functions({}), global_variables({}) {}
void IRModule::addFunction(IRFunction* function) { functions.push_back(function); }
void IRModule::addGlobalVariable(const std::string& name, Operand* variable) { global_variables.insert({name, variable}); }
IRFunction* IRModule::getFunction(const std::string& name) const {
    for(auto& it : functions) {
        if(it->getName() == name) { return it; }
    }
    return nullptr;
}
Operand* IRModule::getGlobalVariable(const std::string& name) const {
    auto it = global_variables.find(name);
    if(it != global_variables.end()) { return it->second; }
    return nullptr;
}

std::vector<IRFunction*> IRModule::getFunctions() const { return functions; }
std::map<std::string, Operand*> IRModule::getGlobalVariables() const { return global_variables; }
std::string IRModule::toString() const {
    std::string result = "";
    if (!global_variables.empty()) {
        result += "GLOBALS:\n";
        for (const auto& [name, operand] : global_variables) {
            result += "  " + name + ": " + operand->toString() + "\n";
        }
        result += "\n";
    }
    for (IRFunction* func : functions) {
        result += func->toString() + "\n";
    }
    return result;
}

void IRInstruction::addPhiOperand(Operand* value, const std::string& label) {
    phi_operands.push_back({value, label});
}