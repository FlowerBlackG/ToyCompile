/*
    TCIR 生成器。

    created on 2022.12.3
*/


#pragma once

#include <tc/core/tcir/SymbolTable.h>
#include <tc/core/tcir/ValueType.h>
#include <tc/core/AstNode.h>

#include <string>
#include <vector>
#include <utility>
#include <iostream>

namespace tc::tcir {

    /**
     * 单条 IR 指令码。
     *
     *   mov   vreg  0   vreg  1  <- 1条指令。
     *    ^     ^    ^    ^    ^   
     *   [0]   [1]  [2]  [3]  [4]
     * 
     */
    typedef std::vector<std::string> IrInstructionCode;

    struct IrGeneratorError {
        std::string msg;
        AstNode* astNode = nullptr;
    };

    class IrGenerator {
    public:
        IrGenerator() {}

    public:

        /**
         * 处理 translationUnit 节点，生成 IR。
         * 
         * @param root 
         * @return int 
         */
        int process(AstNode* root);

        void enableOutputColor() {
            this->outputColorEnabled = true;
        }

        void disableOutputColor() {
            this->outputColorEnabled = false;
        }

        std::vector<IrGeneratorError>& getErrorList() {
            return this->errorList;
        }

        std::vector<IrGeneratorError>& getWarningList() {
            return this->warningList;
        }

        std::vector< IrInstructionCode >& getInstructionCodeList() {
            return this->instructionList;
        }

        /**
         * 清空已经生成的 IR。
         */
        void clear();

    protected:
        VariableDescriptionTable varDescTable;
        GlobalSymbolTable globalSymbolTable;

        BlockSymbolTable* currentBlockSymbolTable = nullptr;

        // 上一个表达式计算结果类型。
        ValueType resultValueType = ValueType::type_void;

        /**
         * 直接触及的符号。
         * 用于实现如 x++ 的运算。
         * 当遇到 x + y 等运算时，应将该符号清空。
         * 
         * @todo 很多地方并没有合理清空该值。
         */
        SymbolBase* directResultSymbol = nullptr;

        /**
         * 当前正在处理的函数。指向 globalSymbolTable 内的成员。
         * 只负责指向，不负责管理内存。
         */
        FunctionSymbol* currentFunction = nullptr;

        int nextLabelId = 1;
        int nextVarId = 1;

        std::vector<IrGeneratorError> errorList;
        std::vector<IrGeneratorError> warningList;

        std::vector< IrInstructionCode > instructionList;

        /**
         * 跳出目标。用于登记循环和 switch 内 break 的跳出目标。
         */
        std::vector< std::string > breakStmtTargets;

        
        /**
         * 跳出目标。用于登记循环内 continue 的跳出目标。
         */
        std::vector< std::string > continueStmtTargets;

        bool outputColorEnabled = true;

    protected:

        void addUnsupportedGrammarError(AstNode* node);

    protected: /* 模块处理函数。 */

        void processTranslationUnit(AstNode* node);
        void processExternalDeclaration(AstNode* node);
        void processFunctionDeclaration(AstNode* node);

        void processCompoundStatement(AstNode* node);
        void processBlockItemList(AstNode* node);
        void processBlockItem(AstNode* node);

        void processStatement(AstNode* node);

        void processSelectionStatement(AstNode* node);
        void processIterationStatement(AstNode* node);

        void processIterationStatementDoWhile(AstNode* statement, AstNode* expression);
        void processIterationStatementWhileLoop(AstNode* expression, AstNode* statement);

        void processIterationStatementForLoop(
            AstNode* expStmtOrDeclaration, 
            AstNode* expStmt, 
            AstNode* expression,
            AstNode* statement
        );

        void processJumpStatement(AstNode* node);

        /**
         * 处理 declaration 节点。
         * 只能处理变量的 declaration。
         * 
         * @param node declaration 节点。
         * @param isInGlobalScope 该语句是否位于全局环境。
         *             位于全局环境时，不能处理算术运算。
         *             不在全局环境时，可能会生成计算语句。
         */
        void processVariableDeclaration(AstNode* node, bool isInGlobalScope);

        void processVariableInitDeclarator(
            AstNode* node, 
            std::vector<TokenKind>& declarationSpecifierTokens, 
            bool isInGlobalScope
        );

        std::string processAssignmentExpression(AstNode* node, bool isInGlobalScope);
        std::string processConditionalExpression(AstNode* node, bool isInGlobalScope);
        std::string processLogicalOrExpression(AstNode* node, bool isInGlobalScope);
        std::string processLogicalAndExpression(AstNode* node, bool isInGlobalScope);
        std::string processInclusiveOrExpression(AstNode* node, bool isInGlobalScope);
        std::string processExclusiveOrExpression(AstNode* node, bool isInGlobalScope);
        std::string processAndExpression(AstNode* node, bool isInGlobalScope);
        std::string processEqualityExpression(AstNode* node, bool isInGlobalScope);
        std::string processRelationalExpression(AstNode* node, bool isInGlobalScope);

        std::string processShiftExpression(AstNode* node, bool isInGlobalScope);
        std::string processAdditiveExpression(AstNode* node, bool isInGlobalScope);
        std::string processMultiplicativeExpression(AstNode* node, bool isInGlobalScope);
        std::string processCastExpression(AstNode* node, bool isInGlobalScope);

        void processExpressionStatement(AstNode* node);
        std::string processExpression(AstNode* node, bool isInGlobalScope);

        std::string processUnaryExpression(AstNode* node, bool isInGlobalScope);

        std::string processPostfixExpression(AstNode* node, bool isInGlobalScope);

        std::string processPrimaryExpression(AstNode* node, bool isInGlobalScope);
        

        /**
         * 处理 declaration_specifiers 节点。
         * 暂时只支持 int 和 void。
         * 
         * @return int 处理过程产生的错误数量。
         */
        int processDeclarationSpecifiers(
            AstNode* node, std::vector< TokenKind >& tokenListContainer
        );

        std::string symbolToIrValueCode(SymbolBase* symbol);


    private:
        IrGenerator(const IrGenerator&) {};

    };

}
