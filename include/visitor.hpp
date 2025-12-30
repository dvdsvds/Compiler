#pragma once

class LiteralExpr;
class VariableExpr;
class BinaryExpr;
class UnaryExpr;
class CallExpr;
class ArrayAccessExpr;
class VarDeclStmt;
class AssignStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class ReturnStmt;
class SendStmt;
class RecvStmt;
class BlockStmt;
class ExprStmt;
class FunctionDecl;
class Program;

class Visitor {
    public:
        virtual ~Visitor() {}
        virtual void visit(LiteralExpr* node) = 0;
        virtual void visit(VariableExpr* node) = 0;
        virtual void visit(BinaryExpr* node) = 0;
        virtual void visit(UnaryExpr* node) = 0;
        virtual void visit(CallExpr* node) = 0;
        virtual void visit(ArrayAccessExpr* node) = 0;
        virtual void visit(VarDeclStmt* node) = 0;
        virtual void visit(AssignStmt* node) = 0;
        virtual void visit(IfStmt* node) = 0;
        virtual void visit(WhileStmt* node) = 0;
        virtual void visit(ForStmt* node) = 0;
        virtual void visit(ReturnStmt* node) = 0;
        virtual void visit(SendStmt* node) = 0;
        virtual void visit(RecvStmt* node) = 0;
        virtual void visit(BlockStmt* node) = 0;
        virtual void visit(ExprStmt* node) = 0;
        virtual void visit(FunctionDecl* node) = 0;
        virtual void visit(Program* node) = 0;
};