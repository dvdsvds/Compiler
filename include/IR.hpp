#pragma once
#include <vector>
#include <map>
#include "token.hpp"

enum class IROpcode {
    ADD, SUB, MUL, DIV, MOD, NEG,
    AND, OR, XOR, NOT, SHL, SHR, SAR,
    EQ, NE, LT, LE, GT, GE,
    LOAD, STORE, ALLOCA,
    LABEL, JUMP, BRANCH,
    CALL, RETURN,
    COPY, PHI,
    SEND, RECV,
    OUT, IN,
};

enum class OperandType {
    VIRTUAL_REG,
    CONSTANT,
    LABEL,
    GLOBAL,
    STACK_SLOT,
};

class Operand {
    private:
        OperandType type;
        int vreg_num;
        int const_value;
        int stack_offset;
        std::string* label_name;
        std::string* global_name;
        Token data_type;

        Operand(OperandType type, int vreg_num, int const_value, int stack_offset, std::string* label_name, std::string* gloabal_name, Token data_type);

    public:
        static Operand* createVirtualReg(int vreg_num, Token data_type);
        static Operand* createConstant(int value, Token data_type);
        static Operand* createLabel(const std::string& label_name);
        static Operand* createGlobal(const std::string& global_name, Token data_type);
        static Operand* createStackSlot(int offset, Token data_type);

        bool isVirtualReg() const;
        bool isConstant() const;
        bool isLabel() const;
        bool isGlobal() const;
        bool isStackSlot() const;

        OperandType getType() const;
        int getVregNum() const;
        int constValue() const;
        int stackOffset() const;
        std::string* labelName() const;
        std::string* globalName() const;
        Token getDataType() const;
        std::string toString() const;
};

class IRInstruction {
    private:
        IROpcode opcode;
        Operand* dest;
        Operand* src1;
        Operand* src2;
        std::string* func_name;
        std::vector<Operand*> args;
        std::vector<std::pair<Operand*, std::string>> phi_operands;

        IRInstruction(IROpcode opcode, Operand* dest, Operand* src1, Operand* src2, std::string* func_name, std::vector<Operand*> args, std::vector<std::pair<Operand*, std::string>> phi_operands);

    public:
        static IRInstruction* createAdd(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createSub(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createMul(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createDiv(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createMod(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createNeg(Operand* dest, Operand* src1);
        static IRInstruction* createAnd(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createOr(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createXor(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createNot(Operand* dest, Operand* src1);
        static IRInstruction* createShl(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createShr(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createSar(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createEq(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createNe(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createLt(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createLe(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createGt(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createGe(Operand* dest, Operand* src1, Operand* src2);
        static IRInstruction* createLoad(Operand* dest, Operand* address);
        static IRInstruction* createStore(Operand* address, Operand* value);
        static IRInstruction* createAlloca(Operand* dest, Operand* size);
        static IRInstruction* createLabel(Operand* label);
        static IRInstruction* createJump(Operand* target);
        static IRInstruction* createReturn(Operand* value);
        static IRInstruction* createCopy(Operand* dest, Operand* src1);
        static IRInstruction* createSend(Operand* value, const std::string& target_func);
        static IRInstruction* createRecv(Operand* dest);
        static IRInstruction* createOut(Operand* value);
        static IRInstruction* createIn(Operand* dest);
        static IRInstruction* createCall(Operand* dest, const std::string& func_name, const std::vector<Operand*>& args);
        static IRInstruction* createPhi(Operand* dest, const std::vector<std::pair<Operand*, std::string>>& phi_operands);
        static IRInstruction* createBranch(Operand* condition, Operand* true_label);

        IROpcode getOpcode() const;
        Operand* getDest() const;
        Operand* getSrc1() const;
        Operand* getSrc2() const;
        std::string* getFuncName() const;
        std::vector<Operand*> getArgs() const;
        std::vector<std::pair<Operand*, std::string>> getPhi() const;
        std::string toString() const;
        void addPhiOperand(Operand* value, const std::string& label);
};

class BasicBlock {
    private:
        std::string label;
        std::vector<IRInstruction*> instructions;
        std::vector<BasicBlock*> successors;
        std::vector<BasicBlock*> predecessors;

    public:
        BasicBlock(const std::string& label);

        void addInstruction(IRInstruction* insert);
        void addSuccessor(BasicBlock* block);
        void addPredecessor(BasicBlock* block);
        
        std::string getLabel() const;
        std::vector<IRInstruction*> getInstructions() const;
        std::vector<BasicBlock*> getSuccessors() const;
        std::vector<BasicBlock*> getPredecessors() const;
        void insertInstructionBefore(int index, IRInstruction* instr);
        void removeInstruction(IRInstruction* instr);
        std::vector<IRInstruction*>& getInstructionsMutable();

        std::string toString() const;
};

class IRFunction {
    private:
        std::string name;
        std::vector<Operand*> parameters;
        Token return_type;
        std::vector<BasicBlock*> basic_blocks;
        BasicBlock* entry_block;

    public:
        IRFunction(const std::string& name, const std::vector<Operand*> parameters, Token return_type);

        void addBasicBlock(BasicBlock* block);
        void setEntryBlock(BasicBlock* entry);
        
        std::string getName() const;
        std::vector<Operand*> getParameters() const;
        Token getReturnType() const;
        std::vector<BasicBlock*> getBasicBlocks() const;
        BasicBlock* getEntryBlock() const;

        std::string toString() const;
};

class IRModule {
    private:
        std::vector<IRFunction*> functions;
        std::map<std::string, Operand*> global_variables;

    public:
        IRModule();

        void addFunction(IRFunction* function);
        void addGlobalVariable(const std::string& name, Operand* variable);

        IRFunction* getFunction(const std::string& name) const;
        Operand* getGlobalVariable(const std::string& name) const;

        std::vector<IRFunction*> getFunctions() const;
        std::map<std::string, Operand*> getGlobalVariables() const;

        std::string toString() const;
};