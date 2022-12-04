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
#include <iostream>

namespace tc::tcir {

    typedef std::vector<std::string> IrInstructionCode;

    struct IrGeneratorError {
        std::string msg;
        AstNode* astNode = nullptr;
    };

    class IrGenerator {
    public:
        int process(AstNode* root);

        void clear();

    protected:
        VariableDescriptionTable varDescTable;
        GlobalSymbolTable globalSymbolTable;

        BlockSymbolTable* currentBlockSymbolTable = nullptr;

        /**
         * 当前正在处理的函数。指向 globalSymbolTable 内的成员。
         * 只负责指向，不负责管理内存。
         */
        FunctionSymbol* currentFunction = nullptr;

        int labelCounter = 0;
        int varCounter = 0;

        std::vector<IrGeneratorError> errorList;

        std::vector< IrInstructionCode > instructionList;

    protected:

        void addUnsupportedGrammarError(AstNode* node);

    protected: /* 模块处理函数。 */

        void processTranslationUnit(AstNode* node);
        void processExternalDeclaration(AstNode* node);
        void processFunctionDeclaration(AstNode* node);

        


    private:
        IrGenerator(const IrGenerator&) {};

    };

}
