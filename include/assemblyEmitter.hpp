#pragma once
#include "IR.hpp"
#include <ostream>
#include <set>

class AssemblyEmitter {
    private:
        std::ostream& output;
        IRModule* module;
        std::map<int, int> reg_assignment_info;
        std::map<int, int> spill_info;
        int offset;
        int label_counter;

        std::map<std::string, int> send_recv_addresses;
        int next_send_recv_addr;
        std::string current_function;

        // Liveness tracking for caller-saved register preservation
        std::map<int, int> vreg_last_use;
        int current_instr_idx;

        bool isCallerSaved(int preg);
        std::set<int> getCallerSavedToPreserve(int call_idx);

        void emitFunction(IRFunction* func);
        void emitBasicBlock(BasicBlock* block);
        void emitInstruction(IRInstruction* instr);
        void allocateReg(IRFunction* func);
        int PhysicalReg(Operand* operand);
        bool needsSpill(int vreg_num);
        int getSpillOffset(int vreg_num);
        void eliminatePHI(IRFunction* func);
        void emitPrologue(IRFunction* func);
        void emitEpilogue(IRFunction* func);

        std::string getOperandReg(Operand* operand);
    public:
        AssemblyEmitter(std::ostream& output);
        void emit(IRModule* module);
};