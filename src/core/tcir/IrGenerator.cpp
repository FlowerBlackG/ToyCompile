/*
    TCIR 生成器。

    created on 2022.12.3
*/

#include <tc/core/tcir/IrGenerator.h>

using namespace std;
using namespace tc;

int tcir::IrGenerator::process(AstNode* root) {

    this->clear();


    // root 一定是 translationUnit

    return this->errorList.size();
}

void tcir::IrGenerator::clear() {
    this->varDescTable.clear();
    this->errorList.clear();
    this->globalSymbolTable.clear();
    this->instructionList.clear();

    if (this->currentBlockSymbolTable) {
        delete this->currentBlockSymbolTable;
        this->currentBlockSymbolTable = nullptr;
    }

    this->labelCounter = 0;
    this->varCounter = 0;
}

void tcir::IrGenerator::addUnsupportedGrammarError(AstNode* node) {
    errorList.emplace_back();
    auto& err = errorList.back();
    err.msg = "not supported: (";
    err.msg += to_string(node->token.row);
    err.msg += ", ";
    err.msg += to_string(node->token.col);
    err.msg += ") ";
    err.msg += node->token.content;
    err.msg += " as ";
    err.msg += node->symbol.name;
}

/* ----------- 模块处理函数。 ------------ */

void tcir::IrGenerator::processTranslationUnit(AstNode* node) {
    if (node->children.size() == 1) {
        
        // translation_unit -> external_declaration
        processExternalDeclaration(node->children[0]);

    } else {
        
        // translation_unit -> translation_unit external_declaration
        processExternalDeclaration(node->children[1]);
        processTranslationUnit(node->children[0]);

    }
}

void tcir::IrGenerator::processExternalDeclaration(AstNode* node) {

    /*
        external_declaration
        : function_definition
        | declaration
    
    */

    AstNode* child = node->children[0];

    if (child->symbolKind == SymbolKind::function_definition) {

        // 函数定义

        processFunctionDeclaration(child);

    } else {

        // declaration 全局变量、结构体、枚举定义

        this->addUnsupportedGrammarError(child);

    }

}

void tcir::IrGenerator::processFunctionDeclaration(AstNode* node) {

    // 为函数创建一张大符号表。
    currentBlockSymbolTable = new BlockSymbolTable;
    currentBlockSymbolTable->parent = currentBlockSymbolTable;
    currentBlockSymbolTable->descTable = &(this->varDescTable);

    if (node->children.size() == 2) {
        /*
            function_definition ->
                declarator compound_statement
        */

        this->addUnsupportedGrammarError(node);
    
    } else if (node->children[0]->symbolKind == SymbolKind::declarator) {

        /*
            function_definition ->
                declarator declaration_list compound_statement
        */

        this->addUnsupportedGrammarError(node);


    } else if (node->children.size() == 3) {

        /*
            function_definition ->

                declaration_specifiers
                如：int
                
                declarator
                如：fibonacci(int a, int b)
                
                compound_statement
                如：{ return f(x - 1); }
        */

       
       // todo


    } else {

        /*
            function_definition ->
                declaration_specifiers declarator 
                declaration_list compound_statement
        */


        this->addUnsupportedGrammarError(node);

    }

    delete currentBlockSymbolTable;
    currentBlockSymbolTable = nullptr;
}

