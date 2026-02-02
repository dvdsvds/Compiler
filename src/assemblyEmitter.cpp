#include "assemblyEmitter.hpp"
#include <iostream>
#include <algorithm>
#include <set>

AssemblyEmitter::AssemblyEmitter(std::ostream& output) : output(output), module(nullptr), label_counter(0), offset(8), next_send_recv_addr(0x00200000) { }

void AssemblyEmitter::emit(IRModule* module) {
    this->module = module;   
    for(const auto& func : module->getFunctions()) {
        emitFunction(func);
    }
}
void AssemblyEmitter::emitFunction(IRFunction* func) {
    current_function = func->getName();

    eliminatePHI(func);
    allocateReg(func);

    output << "FUNCTION_" << func->getName() << ":" << std::endl;
    emitPrologue(func);
    for(const auto& blocks : func->getBasicBlocks()) {
        emitBasicBlock(blocks);
    }
    emitEpilogue(func);
}
void AssemblyEmitter::emitBasicBlock(BasicBlock* block) {
    output << block->getLabel() << ":" << std::endl; 
    for(const auto& instr : block->getInstructions()) {
        emitInstruction(instr);
    }
}
std::string AssemblyEmitter::getOperandReg(Operand* operand) {
    if(operand->isConstant()) {
        int imm = operand->constValue();
        output << "mov r_temp, " << imm << std::endl; 
        return "r_temp";
    } else if(operand->isVirtualReg()) {
        int reg = PhysicalReg(operand);
        return "r" + std::to_string(reg);
    }
    throw std::runtime_error("Unknown operand type");
}
void AssemblyEmitter::emitInstruction(IRInstruction* instr) {
    // std::cerr << "Processing: " << instr->toString() << std::endl;
    switch (instr->getOpcode()) { 
        case IROpcode::ADD: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "add r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::SUB: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "sub r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::MUL: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "mul r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::DIV: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "div r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::MOD: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "mod r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::AND: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "and r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::OR: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "or r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::XOR: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "xor r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::SHL: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "shl r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::SHR: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "shr r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::EQ: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "cmp r" << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "bjmp EQ, " << set_true << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::NE: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "cmp " << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "bjmp NE, " << set_true << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::LT: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "cmp " << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "bjmp LT, " << set_true << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::LE: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "cmp " << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "bjmp LE, " << set_true << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::GT: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "cmp " << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "bjmp GT, " << set_true << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::GE: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "cmp " << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "bjmp GE, " << set_true << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::LOAD: {
            int rd = PhysicalReg(instr->getDest());
            int address = PhysicalReg(instr->getSrc1());
            output << "loadw r" << rd << ", " << "0(r" << address << ")" << std::endl;
            break;
        }
        case IROpcode::STORE: {
            int address = PhysicalReg(instr->getDest());
            int value = PhysicalReg(instr->getSrc1());
            output << "storew r" << value << ", " << "0(r" << address << ")" << std::endl;
            break;
        }
        case IROpcode::ALLOCA: {
            int rd = PhysicalReg(instr->getDest());
            Operand* size_op = instr->getSrc1();
            output << "csrr r_temp, SP" << std::endl;
            if(size_op->isConstant()) {
                int size = size_op->constValue();
                output << "sub r_temp, r_temp, " << size << std::endl;
            } else {
                int size_reg = PhysicalReg(size_op);
                output << "sub r_temp, r_temp, r" << size_reg << std::endl;
            }
            output << "csrw r_temp, SP" << std::endl;
            output << "mov r" << rd << ", r_temp" << std::endl;
            break;
        }
        case IROpcode::JUMP: {
            std::string* target_label = instr->getSrc1()->labelName();
            output << "jmp " << *target_label << std::endl;
            break;
        }
        case IROpcode::BRANCH: {
            int condition = PhysicalReg(instr->getSrc1());
            std::string* target_label = instr->getSrc2()->labelName();
            output << "cmp r" << condition << ", r0" << std::endl;
            output << "bjmp NE, " << *target_label << std::endl;
            break;
        }
        case IROpcode::CALL: {
            std::string* funcName = instr->getFuncName();
            std::vector<Operand*> args = instr->getArgs();

            int count = std::min(8, static_cast<int>(args.size()));
            for(int i = 0; i < count; i++) {
                std::string arg_reg = getOperandReg(args[i]);
                output << "mov r" << 4 + i << ", " << arg_reg << std::endl;
            }

            if(args.size() > 8) {  
                output << "csrr r_temp, SP" << std::endl;
                for(int i = 8; i < args.size(); i++) {
                    std::string arg_reg = getOperandReg(args[i]);
                    int offset = -(i - 7) * 4;
                    output << "storew r" << arg_reg << ", " << offset << "(r_temp)" << std::endl;
                }
                int stack_size = (args.size() - 8) * 4;
                output << "sub r_temp, r_temp, " << stack_size << std::endl;
                output << "csrw r_temp, SP" << std::endl;
            }
            
            output << "call " << *funcName << std::endl;

            if(args.size() > 8) {
                output << "csrr r_temp, SP" << std::endl;
                int stack_size = (args.size() - 8) * 4;
                output << "add r_temp, r_temp, " << stack_size << std::endl;
                output << "csrw r_temp, SP" << std::endl;
            }

            if(instr->getDest() != nullptr) {
                int rd = PhysicalReg(instr->getDest());
                output << "mov r" << rd << ", r3" << std::endl;
            }
            break;
        }
        case IROpcode::RETURN: {
            if(instr->getSrc1() == nullptr) {
                output << "ret" << std::endl;
            } else {
                int rs1 = PhysicalReg(instr->getSrc1());
                output << "mov r3, r" << rs1 << std::endl;
                output << "ret" << std::endl;
            }
            break;
        }
        case IROpcode::SEND: {
            int value = PhysicalReg(instr->getSrc1());
            std::string* target_func = instr->getFuncName();
            output << "storew r" << value << ", " << next_send_recv_addr << std::endl;
            next_send_recv_addr += 4;
            break;
        }
        case IROpcode::RECV: {
            int rd = PhysicalReg(instr->getDest());
            next_send_recv_addr -= 4;
            output << "loadw r" << rd << ", " << next_send_recv_addr << std::endl;
            break;
        }
        case IROpcode::OUT: {
            int rs1 = PhysicalReg(instr->getSrc1());
            output << "csrr r_temp, SP" << std::endl; 
            output << "storew r" << rs1 << ", " << offset << "(r_temp)" << std::endl;  
            offset += 4;
            break;
        }
        case IROpcode::IN: {
            int rd = PhysicalReg(instr->getDest());
            offset -= 4;
            output << "csrr r_temp, SP" << std::endl;
            output << "loadw r" << rd << ", " << offset << "(r_temp)" << std::endl;
            break;
        }
        case IROpcode::NEG: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            output << "neg r" << rd << ", " << rs1 << std::endl;
            break;
        }
        case IROpcode::NOT: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            output << "not r" << rd << ", " << rs1 << std::endl;
            break;
        }
        case IROpcode::SAR: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            std::string rs2 = getOperandReg(instr->getSrc2());
            output << "sar r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::LABEL: break;
        case IROpcode::COPY: {
            int rd = PhysicalReg(instr->getDest());

            if(instr->getSrc1()->isConstant()) {
                int imm = instr->getSrc1()->constValue();
                output << "mov r" << rd << ", " << imm << std::endl;
            } else if(instr->getSrc1()->isVirtualReg()) {
                int rs1 = PhysicalReg(instr->getSrc1());
                output << "mov r" << rd << ", r" << rs1 << std::endl;
            }
            break;
        }
        case IROpcode::PHI: break;
        default: break;
    }
}

struct LiveRange {
    int vreg_num;
    int start;
    int end;
};

void AssemblyEmitter::allocateReg(IRFunction* func) {
    auto params = func->getParameters();
    for(int i = 0; i < params.size() && i < 8; i++) {
        int vreg_num = params[i]->getVregNum();
        reg_assignment_info[vreg_num] = 4 + i;
    }

    std::map<int, int> vreg_def;
    std::map<int, int> vreg_last_use;
    std::vector<LiveRange> live_ranges;

    int instr_idx = 0;
    for(const auto& block : func->getBasicBlocks()) {
        for(const auto& instr : block->getInstructions()) {
            if(instr->getDest() != nullptr && instr->getDest()->isVirtualReg()) {
                vreg_def[instr->getDest()->getVregNum()] = instr_idx;
            }
            if(instr->getSrc1() != nullptr && instr->getSrc1()->isVirtualReg()) {
                vreg_last_use[instr->getSrc1()->getVregNum()] = instr_idx;
            }
            if(instr->getSrc2() != nullptr && instr->getSrc2()->isVirtualReg()) {
                vreg_last_use[instr->getSrc2()->getVregNum()] = instr_idx;
            }
            instr_idx++;

        }
    }
    for(const auto& vreg : vreg_def) {
        live_ranges.push_back({vreg.first, vreg.second, vreg_last_use[vreg.first]});
    }

    std::sort(live_ranges.begin(), live_ranges.end(),
        [](const LiveRange& a, const LiveRange& b) {
            return a.start < b.start;
        });

    std::set<int> available_regs = {1, 2, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29 ,30, 31};
    std::map<int, int> vreg_to_preg;
    std::vector<LiveRange> active;
    for(const auto& lr : live_ranges) {
        for(int i = active.size() - 1; i >= 0; i--) {
            if(active[i].end < lr.start) {
                available_regs.insert(vreg_to_preg[active[i].vreg_num]);
                active.erase(active.begin() + i);
            }
        }
        if(!available_regs.empty()) {
            int preg = *available_regs.begin();
            available_regs.erase(available_regs.begin());
            vreg_to_preg[lr.vreg_num] = preg;  
            active.push_back(lr);
        } else {
            spill_info[lr.vreg_num] = -(spill_info.size() + 1) * 4;
        }
    }

    for(const auto& mapping : vreg_to_preg) {
        reg_assignment_info[mapping.first] = mapping.second;
    }
}
int AssemblyEmitter::PhysicalReg(Operand* operand) {
    if(operand == nullptr) {
        throw std::runtime_error("Operand is nullptr");
    }

    if(!operand->isVirtualReg()) {
        throw std::runtime_error("Operand is not a virtual register");
    }

    int vreg_num = operand->getVregNum();
    if(reg_assignment_info.find(vreg_num) == reg_assignment_info.end()) {
        throw std::runtime_error("Virtual register not allocated: v" + std::to_string(vreg_num));
    }

    return reg_assignment_info[vreg_num];
}
bool AssemblyEmitter::needsSpill(int vreg_num) {
    if(spill_info.find(vreg_num) != spill_info.end()) {
        return true;
    }
    return false;
}
int AssemblyEmitter::getSpillOffset(int vreg_num) {
    if(spill_info.find(vreg_num) == spill_info.end()) {
        throw std::runtime_error("Virtual register not spilled: v" + std::to_string(vreg_num));
    }
    return spill_info[vreg_num];
}
void AssemblyEmitter::eliminatePHI(IRFunction* func) {
    for(auto& block : func->getBasicBlocks()) {
        std::vector<IRInstruction*> phi_to_remove;
        for(const auto& instr : block->getInstructions()) {
            if(instr->getOpcode() == IROpcode::PHI) {
                Operand* dest = instr->getDest();
                std::vector<std::pair<Operand*, std::string>> phi_operands = instr->getPhi();
                for(const auto& phi_pair : phi_operands) {
                    std::vector<BasicBlock*> predecessors = block->getPredecessors();
                    BasicBlock* pred_block = nullptr;
                    for(auto pred : predecessors) {
                        if(pred->getLabel() == phi_pair.second) {
                            pred_block = pred;
                            break;
                        }
                    }
                    if(pred_block != nullptr) {
                        IRInstruction* copy_instr = IRInstruction::createCopy(instr->getDest(), phi_pair.first);
                        pred_block->insertInstructionBefore(pred_block->getInstructions().size() - 1, copy_instr);
                    }
                }
                phi_to_remove.push_back(instr);
            }
        }
        for(auto phi : phi_to_remove) {
            block->removeInstruction(phi);
        }
    }
}
void AssemblyEmitter::emitPrologue(IRFunction* func) {
    std::set<int> used_callee_saved = {};
    for(const auto& mapping : reg_assignment_info) {
        int vreg = mapping.first;
        int preg = mapping.second;
        if(preg >= 16 && preg <= 23) {
            used_callee_saved.insert(preg);
        }
    }

    for(const auto& saved_reg : used_callee_saved) {
        output << "csrr r_temp, SP" << std::endl;
        output << "storew r" << saved_reg << ", -4(r_temp)" << std::endl;
        output << "sub r_temp, r_temp, 4" << std::endl;
        output << "csrw r_temp, SP" << std::endl;
    }

    if(!spill_info.empty()) {
        int size = spill_info.size() * 4;
        output << "csrr r_temp, SP" << std::endl;
        output << "sub r_temp, r_temp, " << size << std::endl;
        output << "csrw r_temp, SP" << std::endl;
    }
}
void AssemblyEmitter::emitEpilogue(IRFunction* func) {
    std::set<int> used_callee_saved = {};
    for(const auto& mapping : reg_assignment_info) {
        int vreg = mapping.first;
        int preg = mapping.second;
        if(preg >= 16 && preg <= 23) {
            used_callee_saved.insert(preg);
        }
    }

    if(!spill_info.empty()) {
        int size = spill_info.size() * 4;
        output << "csrr r_temp, SP" << std::endl;
        output << "add r_temp, r_temp, " << size << std::endl;
        output << "csrw r_temp, SP" << std::endl;
    }

    std::vector<int> regs(used_callee_saved.begin(), used_callee_saved.end());
    for(int i = regs.size() - 1; i >= 0; i--) {
        output << "csrr r_temp, SP" << std::endl;
        output << "loadw r" << regs[i] << ", 0(r_temp)" << std::endl;
        output << "add r_temp, r_temp, 4" << std::endl;
        output << "csrw r_temp, SP" << std::endl;

    }
}