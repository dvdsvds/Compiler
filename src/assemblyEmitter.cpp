#include "assemblyEmitter.hpp"
#include <cstdint>
#include <algorithm>
#include <set>

AssemblyEmitter::AssemblyEmitter(std::ostream& output) : output(output), module(nullptr), label_counter(0), offset(-4), next_send_recv_addr(0x00200000) { }

void AssemblyEmitter::emit(IRModule* module) {
    this->module = module;   

    output << "_start:" << std::endl;
    output << "mov r_temp, 0x1000" << std::endl;
    output << "csrw IVTBR, r_temp" << std::endl;
    output << "mov r29, _syscall_handler" << std::endl;
    output << "storew r29, 20(r_temp)" << std::endl;

    output << "mov r_temp, 1" << std::endl;
    output << "csrw STATUS, r_temp" << std::endl;
    output << "mov r_temp, 0x20" << std::endl;
    output << "csrw IMASK, r_temp" << std::endl;

    output << "mov r_temp, 0xFFFFC" << std::endl;
    output << "csrw SP, r_temp" << std::endl;
    output << "call main" << std::endl;
    output << "hlt" << std::endl;
    output << std::endl;

    output << "_syscall_handler:" << std::endl;
    output << "cmp r28, 1" << std::endl;
    output << "bjmp EQ, _print_int" << std::endl;
    output << "cmp r28, 2" << std::endl;
    output << "bjmp EQ, _print_bool" << std::endl;
    output << "cmp r28, 3" << std::endl;
    output << "bjmp EQ, _print_str" << std::endl;
    output << "jmp _syscall_end" << std::endl;
    output << "_print_int:" << std::endl;
    output << "mov r_temp, 0x100" << std::endl;
    output << "storew r1, 0(r_temp)" << std::endl;
    output << "jmp _syscall_end" << std::endl;
    output << "_print_bool:" << std::endl;
    output << "mov r_temp, 0x104" << std::endl;
    output << "storew r1, 0(r_temp)" << std::endl;
    output << "jmp _syscall_end" << std::endl;
    output << "_print_str:" << std::endl;
    output << "mov r_temp, 0x108" << std::endl;
    output << "storew r1, 0(r_temp)" << std::endl;
    output << "_syscall_end:" << std::endl;
    output << "iret" << std::endl;
    output << std::endl;

    for(const auto& func : module->getFunctions()) {
        emitFunction(func);
    }

    for(auto& [label, value] : module->getStringLiterals()) {
        output << label << ":" << std::endl;
        std::string data = value + '\0';
        for(size_t i = 0; i < data.size(); i += 4) {
            uint32_t word = 0;
            for(int j = 0; j < 4 && (i + j) < data.size(); j++) {
                word |= (static_cast<uint8_t>(data[i + j]) << (j * 8));
            }
            output << ".word " << word << std::endl;
        }
    }
}
void AssemblyEmitter::emitFunction(IRFunction* func) {
    current_function = func->getName();
    current_instr_idx = 0;

    eliminatePHI(func);
    allocateReg(func);

    output << "FUNCTION_" << func->getName() << ":" << std::endl;
    auto blocks = func->getBasicBlocks();
    for(int i = 0; i < blocks.size(); i++) {
        if(i == 0) {
            output << blocks[i]->getLabel() << ":" << std::endl;
            emitPrologue(func);
            for(const auto& instr : blocks[i]->getInstructions()) {
                emitInstruction(instr);
                current_instr_idx++;
            }
        } else {
            emitBasicBlock(blocks[i]);
        }
    }
    emitEpilogue(func);
}
void AssemblyEmitter::emitBasicBlock(BasicBlock* block) {
    output << block->getLabel() << ":" << std::endl;
    for(const auto& instr : block->getInstructions()) {
        emitInstruction(instr);
        current_instr_idx++;
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
    } else if(operand->isGlobal()) {
        output << "mov r_temp, " << *operand->globalName() << std::endl;
        return "r_temp";
    }
    throw std::runtime_error("Unknown operand type");
}
void AssemblyEmitter::emitInstruction(IRInstruction* instr) {
    switch (instr->getOpcode()) { 
        case IROpcode::ADD: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "add r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::SUB: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "sub r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::MUL: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "mul r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::DIV: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "div r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::MOD: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "mod r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::AND: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "and r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::OR: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "or r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::XOR: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "xor r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::SHL: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "shl r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::SHR: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "shr r" << rd << ", " << rs1 << ", " << rs2 << std::endl;
            break;
        }
        case IROpcode::EQ: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "cmp " << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "bjmp EQ, " << set_true << std::endl;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::NE: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "cmp " << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "bjmp NE, " << set_true << std::endl;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::LT: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "cmp " << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "bjmp LT, " << set_true << std::endl;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::LE: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "cmp " << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "bjmp LE, " << set_true << std::endl;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::GT: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "cmp " << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "bjmp GT, " << set_true << std::endl;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::GE: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1, rs2;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                rs1 = "r_temp";
            } else {
                rs1 = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                rs2 = "r29";
            } else {
                rs2 = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "cmp " << rs1 << ", " << rs2 << std::endl;
            std::string set_true = "set_true_" + std::to_string(label_counter);
            std::string done = "done_" + std::to_string(label_counter);
            label_counter++;
            output << "bjmp GE, " << set_true << std::endl;
            output << "mov r" << rd << ", 0" << std::endl;
            output << "jmp " << done << std::endl;
            output << set_true << ":" << std::endl;
            output << "mov r" << rd << ", 1" << std::endl;
            output << done << ":" << std::endl;
            break;
        }
        case IROpcode::LOAD: {
            int rd = PhysicalReg(instr->getDest());
            if(instr->getSrc1()->isConstant()) {
                output  << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                output << "loadw r" << rd << ", 0(r_temp)" << std::endl;
            } else {
                int address = PhysicalReg(instr->getSrc1());
                output << "loadw r" << rd << ", " << "0(r" << address << ")" << std::endl;
            }
            break;
        }
        case IROpcode::STORE: {
            std::string addr_reg;
            if(instr->getSrc1()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc1()->constValue() << std::endl;
                addr_reg = "r_temp";
            } else {
                addr_reg = "r" + std::to_string(PhysicalReg(instr->getSrc1()));
            }
            std::string value_str;
            if(instr->getSrc2()->isConstant()) {
                output << "mov r29, " << instr->getSrc2()->constValue() << std::endl;
                value_str = "r29";
            } else if(instr->getSrc2()->isGlobal()) {
                output << "mov r29, " << *instr->getSrc2()->globalName() << std::endl;
                value_str = "r29";
            } else {
                value_str = "r" + std::to_string(PhysicalReg(instr->getSrc2()));
            }
            output << "storew " << value_str << ", " << "0(" << addr_reg << ")" << std::endl;
            break;
        }
        case IROpcode::ALLOCA: {
            int rd = PhysicalReg(instr->getDest());
            Operand* size_op = instr->getSrc1();
            output << "csrr r_temp, SP" << std::endl;
            if(size_op->isConstant()) {
                int size = size_op->constValue();
                output << "mov r29, " << size << std::endl;;
                output << "sub r_temp, r_temp, r29" << std::endl;
            } else {
                int size_reg = PhysicalReg(size_op);
                output << "sub r_temp, r_temp, r" << size_reg << std::endl;
            }
            output << "csrw SP, r_temp" << std::endl;
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

            std::set<int> regs_to_save = getCallerSavedToPreserve(current_instr_idx);

            for(int reg : regs_to_save) {
                output << "push r" << reg << std::endl;
            }

            int count = std::min(8, static_cast<int>(args.size()));
            for(int i = 0; i < count; i++) {
                std::string arg_reg = getOperandReg(args[i]);
                output << "push " << arg_reg << std::endl;
            }
            for(int i = count - 1; i >= 0; i--) {
                output << "pop r" << 4 + i << std::endl;
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

            output << "push r30" << std::endl;
            output << "call " << *funcName << std::endl;
            output << "pop r30" << std::endl;

            if(args.size() > 8) {
                output << "csrr r_temp, SP" << std::endl;
                int stack_size = (args.size() - 8) * 4;
                output << "add r_temp, r_temp, " << stack_size << std::endl;
                output << "csrw r_temp, SP" << std::endl;
            }

            std::vector<int> regs_vec(regs_to_save.begin(), regs_to_save.end());
            for(int i = regs_vec.size() - 1; i >= 0; i--) {
                output << "pop r" << regs_vec[i] << std::endl;
            }

            if(instr->getDest() != nullptr) {
                int rd = PhysicalReg(instr->getDest());
                output << "mov r" << rd << ", r3" << std::endl;
            }
            break;
        }
        case IROpcode::CALL_PTR: {
            Operand* fp = instr->getSrc1();
            std::vector<Operand*> args = instr->getArgs();

            std::set<int> regs_to_save = getCallerSavedToPreserve(current_instr_idx);
            for(int reg : regs_to_save) {
                output << "push r" << reg << std::endl;
            }

            int count = std::min(8, static_cast<int>(args.size()));
            for(int i = 0; i < count; i++) {
                std::string arg_reg = getOperandReg(args[i]);
                output << "push " << arg_reg << std::endl;
            }
            for(int i = count - 1; i >= 0; i--) {
                output << "pop r" << 4 + i << std::endl;
            }

            std::string fp_reg = getOperandReg(fp);
            output << "push r30" << std::endl;
            output << "callr " << fp_reg << std::endl;
            output << "pop r30" << std::endl;

            std::vector<int> regs_vec(regs_to_save.begin(), regs_to_save.end());
            for(int i = regs_vec.size() - 1; i >= 0; i--) {
                output << "pop r" << regs_vec[i] << std::endl;
            }

            if(instr->getDest() != nullptr) {
                int rd = PhysicalReg(instr->getDest());
                output << "mov r" << rd << ", r3" << std::endl;
            }
            break;
        }
        case IROpcode::RETURN: {
            output << "csrw SP, r30" << std::endl;
            output << "csrr r_temp, SP" << std::endl;
            output << "loadw r30, 0(r_temp)" << std::endl;
            output << "mov r29, 4" << std::endl;
            output << "add r_temp, r_temp, r29" << std::endl;
            output << "csrw SP, r_temp" << std::endl;
            if(instr->getSrc1() == nullptr) {
                output << "ret" << std::endl;
            } else {
                std::string rs1 = getOperandReg(instr->getSrc1());
                output << "mov r3, " << rs1 << std::endl;
                output << "ret" << std::endl;
            }
            break;
        }
        case IROpcode::SEND: {
            std::string value_reg = getOperandReg(instr->getSrc1());
            if(value_reg == "r_temp") {
                output << "mov r29, r_temp" << std::endl;
                value_reg = "r29";
            }
            std::string* target_func = instr->getFuncName();
            output << "mov r_temp, " << next_send_recv_addr << std::endl;
            output << "storew " << value_reg << ", 0(r_temp)" << std::endl;
            next_send_recv_addr += 4;
            break;
        }
        case IROpcode::RECV: {
            int rd = PhysicalReg(instr->getDest());
            next_send_recv_addr -= 4;
            output << "mov r_temp, " << next_send_recv_addr << std::endl;
            output << "loadw r" << rd << ", 0(r_temp)" << std::endl;
            break;
        }
        case IROpcode::OUT: {
            output << "csrr r30, SP" << std::endl;
            std::string rs1 = getOperandReg(instr->getSrc1());
            output << "storew " << rs1 << ", " << offset << "(r30)" << std::endl;
            offset -= 4;
            break;
        }
        case IROpcode::IN: {
            int rd = PhysicalReg(instr->getDest());
            offset += 4;
            output << "csrr r_temp, SP" << std::endl;
            output << "loadw r" << rd << ", " << offset << "(r_temp)" << std::endl;
            break;
        }
        case IROpcode::NEG: {
            int rd = PhysicalReg(instr->getDest());
            std::string rs1 = getOperandReg(instr->getSrc1());
            output << "sub r" << rd << ", r0, " << rs1 << std::endl;
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
        case IROpcode::PRINT: {
            Operand* src = instr->getSrc1();
            if(src->isGlobal()) {
                output << "mov r1, " << *src->globalName() << std::endl;
            } else {
                std::string rs1 = getOperandReg(src);
                output << "mov r1, " << rs1 << std::endl;
            }
            
            Token dataType = instr->getSrc1()->getDataType();
            if(dataType == Token::BOOL) {
                output << "mov r28, 2" << std::endl;
            } else if(dataType == Token::STRING) {
                output << "mov r28, 3" << std::endl;
            } else {
                output << "mov r28, 1" << std::endl;
            }
            output << "syscall" << std::endl;
            break;
        }
        case IROpcode::CSRR_OP: {
            int rd = PhysicalReg(instr->getDest());
            int csr = instr->getSrc1()->constValue();
            static const char* names[] = {
                "EPC","CAUSE","STATUS","IVTBR","IMASK","IPENDING",
                "SEPC","SSTATUS","SCRATCH","CYCLE",
                "TPERIOD","TCONTROL","TCOUNTER","EFLAGS","SP"
            };
            std::string csr_name = (csr >= 0 && csr < 15) ? names[csr] : std::to_string(csr);
            output << "csrr r" << rd << ", " << csr_name << std::endl;
            break;
        }
        case IROpcode::CSRW_OP: {
            int csr = instr->getSrc1()->constValue();
            static const char* names[] = {
                "EPC","CAUSE","STATUS","IVTBR","IMASK","IPENDING",
                "SEPC","SSTATUS","SCRATCH","CYCLE",
                "TPERIOD","TCONTROL","TCOUNTER","EFLAGS","SP"
            };
            std::string csr_name = (csr >= 0 && csr < 15) ? names[csr] : std::to_string(csr);
            if(instr->getSrc2()->isConstant()) {
                output << "mov r_temp, " << instr->getSrc2()->constValue() << std::endl;
                output << "csrw " << csr_name << ", r_temp" << std::endl;
            } else {
                int rs = PhysicalReg(instr->getSrc2());
                output << "csrw " << csr_name << ", r" << rs << std::endl;
            }
            break;
        }
        case IROpcode::PUSH_OP: {
            int rs = instr->getSrc1()->constValue();
            output << "push r" << rs << std::endl;
            break;
        }
        case IROpcode::POP_OP: {
            int rd = instr->getSrc1()->constValue();
            output << "pop r" << rd << std::endl;
            break;
        }
        case IROpcode::IRET_OP: {
            output << "iret" << std::endl;
            break;
        }
        default: break;
    }
}

struct LiveRange {
    int vreg_num;
    int start;
    int end;
};

void AssemblyEmitter::allocateReg(IRFunction* func) {
    reg_assignment_info.clear();
    vreg_last_use.clear();
    spill_info.clear();

    auto params = func->getParameters();
    for(int i = 0; i < params.size() && i < 8; i++) {
        int vreg_num = params[i]->getVregNum();
        reg_assignment_info[vreg_num] = 4 + i;
    }
    std::map<int, int> vreg_def;
    std::vector<LiveRange> live_ranges;

    int instr_idx = 0;
    for(const auto& block : func->getBasicBlocks()) {
        for(const auto& instr : block->getInstructions()) {
            if(instr->getDest() != nullptr && instr->getDest()->isVirtualReg()) {
                int vreg = instr->getDest()->getVregNum();
                if(vreg_def.find(vreg) == vreg_def.end()) {
                    vreg_def[vreg] = instr_idx; 
                } else {
                    vreg_last_use[vreg] = instr_idx;
                }
            }
            if(instr->getSrc1() != nullptr && instr->getSrc1()->isVirtualReg()) {
                vreg_last_use[instr->getSrc1()->getVregNum()] = instr_idx;
            }
            if(instr->getSrc2() != nullptr && instr->getSrc2()->isVirtualReg()) {
                vreg_last_use[instr->getSrc2()->getVregNum()] = instr_idx;
            }
            for(Operand* arg : instr->getArgs()) {
                if(arg != nullptr && arg->isVirtualReg()) {
                    vreg_last_use[arg->getVregNum()] = instr_idx;
                }
            }
            instr_idx++;

        }
    }
    for(const auto& vreg : vreg_def) {
        live_ranges.push_back({vreg.first, vreg.second, vreg_last_use[vreg.first]});
    }

    int idx = 0;
    std::map<std::string, int> block_start_idx;
    std::map<std::string, int> block_end_idx;
    for(const auto& block : func->getBasicBlocks()) {
        block_start_idx[block->getLabel()] = idx;
        idx += block->getInstructions().size();
        block_end_idx[block->getLabel()] = idx - 1;
    }

    for(const auto& block : func->getBasicBlocks()) {
        for(auto succ : block->getSuccessors()) {
            if(block_start_idx.count(succ->getLabel()) && block_start_idx[succ->getLabel()] <= block_start_idx[block->getLabel()]) {
                int loop_start = block_start_idx[succ->getLabel()];
                int loop_end = block_end_idx[block->getLabel()];
                for(auto& lr : live_ranges) {
                    if(lr.start < loop_start && lr.end >= loop_start) {
                        if(lr.end < loop_end) lr.end = loop_end;
                    }
                }
            }
        }
    }

    std::sort(live_ranges.begin(), live_ranges.end(),
        [](const LiveRange& a, const LiveRange& b) {
            return a.start < b.start;
        });

    for(const auto& lr : live_ranges) {
        vreg_last_use[lr.vreg_num] = lr.end;
    }

    for(const auto& block : func->getBasicBlocks()) {
        for(auto succ : block->getSuccessors()) {
            if(block_start_idx.count(succ->getLabel()) && block_start_idx[succ->getLabel()] <= block_start_idx[block->getLabel()]) {
                int loop_start = block_start_idx[succ->getLabel()];
                int loop_end = block_end_idx[block->getLabel()];
                for(auto& kv : vreg_last_use) {
                    if(kv.second >= loop_start && kv.second <= loop_end) {
                        kv.second = loop_end;
                    }
                }
            }
        }
    }

    std::set<int> available_regs = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27};
    for(int i = 0; i < params.size() && i < 8; i++) {
        available_regs.erase(4 + i);
    }
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
    output << "csrr r_temp, SP" << std::endl;
    output << "storew r30, -4(r_temp)" << std::endl;
    output << "mov r29, 4" << std::endl;
    output << "sub r_temp, r_temp, r29" << std::endl;
    output << "csrw SP, r_temp" << std::endl;
    output << "csrr r30, SP" << std::endl;
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
bool AssemblyEmitter::isCallerSaved(int preg) {
    return (preg >= 1 && preg <= 15) || (preg >= 24 && preg <= 28);
}
std::set<int> AssemblyEmitter::getCallerSavedToPreserve(int call_idx) {
    std::set<int> regs_to_save;
    for(const auto& mapping : reg_assignment_info) {
        int vreg = mapping.first;
        int preg = mapping.second;
        if(isCallerSaved(preg) && vreg_last_use.count(vreg) && vreg_last_use[vreg] > call_idx) {
            regs_to_save.insert(preg);
        }
    }
    return regs_to_save;
}