/*
    TCIR 生成器。

    created on 2022.12.3
*/

#include <tc/core/tcir/IrGenerator.h>

using namespace std;
using namespace tc;

int tcir::IrGenerator::process(AstNode* root) {

    this->clear();


    this->processTranslationUnit(root);

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

    AstNode* firstToken = node;
    while (firstToken->symbolType == grammar::SymbolType::NON_TERMINAL) {
        firstToken = firstToken->children[0];
    }

    err.msg = "not supported: (";
    err.msg += to_string(firstToken->token.row);
    err.msg += ", ";
    err.msg += to_string(firstToken->token.col);
    err.msg += ") ";
    err.msg += firstToken->token.content;
    err.msg += " as ";
    err.msg += node->symbol.name;
}

/* ----------- 模块处理函数。 ------------ */

void tcir::IrGenerator::processTranslationUnit(AstNode* node) {

    cout << "processing translation_unit" << endl;

    if (node->children.size() == 1) {
        
        // translation_unit -> external_declaration
        processExternalDeclaration(node->children[0]);

    } else {
        
        // translation_unit -> translation_unit external_declaration
        processTranslationUnit(node->children[0]);
        processExternalDeclaration(node->children[1]);

    }
}

void tcir::IrGenerator::processExternalDeclaration(AstNode* node) {

    cout << "processing ExternalDeclaration" << endl;
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


        cout << (int) child->symbol.symbolKind << endl;
        cout << (int) child->symbolKind << endl;

        this->addUnsupportedGrammarError(child);

    }

}


void tcir::IrGenerator::processFunctionDeclaration(AstNode* node) {
    cout << "processing FunctionDeclaration" << endl;


    if (node->children.size() == 3) {

        /*
            function_definition ->

                declaration_specifiers
                如：int
                
                declarator
                如：fibonacci(int a, int b)
                
                compound_statement
                如：{ return f(x - 1); }
        */

       
        AstNode* declarationSpecifiersNode = node->children[0];
        AstNode* declaratorNode = node->children[1];
        AstNode* compoundStmtNode = node->children[2];

        // 处理返回类型。
        // 仅允许 int 和 void。

        vector<TokenKind> declarationSpecifierTokens;
        int resCode = this->processDeclarationSpecifiers(
            declarationSpecifiersNode, declarationSpecifierTokens
        );

        if (resCode > 0) {
            
            return;
        }

        // 处理函数名和参数表。
        if (declaratorNode->children.size() != 1) {
            this->addUnsupportedGrammarError(declaratorNode->children[0]);
            return;
        }

        // 仅支持 direct_declarator -> direct_declarator '(' parameter_type_list ')'
        //    以及 direct_declarator -> direct_declarator '(' ')'
        AstNode* directDeclarator = declaratorNode->children[0];

        vector<FunctionParamSymbol> functionParams;

        // 过滤不支持的类型，同时获取参数表。
        if (directDeclarator->children.size() == 3
            && directDeclarator->children[1]->symbolType == grammar::SymbolType::TERMINAL
            && directDeclarator->children[1]->tokenKind == TokenKind::l_paren
        ) {
            
            // do nothing
        
        } else if (directDeclarator->children.size() == 4
            && directDeclarator->children[2]->symbolKind == SymbolKind::parameter_type_list
        ) do { // 采用 do while(0) 结构，更好中途跳出。

            AstNode* parameterTypeList = directDeclarator->children[2];
            if (parameterTypeList->children.size() != 1) {
                // 带变长参数的，不支持。
                this->addUnsupportedGrammarError(parameterTypeList);
                break;
            } 

            AstNode* paramList = parameterTypeList->children[0];
            vector<AstNode*> paramDeclarations;

            while (paramList->children.size() > 1) {
                paramDeclarations.push_back(paramList->children[2]);
                paramList = paramList->children[0];
            }

            // 现在，所谓 "paramList" 其实变成了 parameter_declaration
            paramDeclarations.push_back(paramList->children[0]);

            // 反向遍历。
            for (auto it = paramDeclarations.rbegin(); it != paramDeclarations.rend(); it++) {
                auto& paramDecl = *it;
                
                if (paramDecl->children.size() == 1) {
                    // parameter_declaration -> declaration_specifiers
                    this->addUnsupportedGrammarError(paramDecl);
                    continue;
                }

                // 读取数据类型。
                vector<TokenKind> resContainer;
                int resCode = this->processDeclarationSpecifiers(
                    paramDecl->children[0], resContainer
                );

                if (resCode) {
                    
                    continue;
                }


                if (paramDecl->children[1]->symbolKind == SymbolKind::abstract_declarator) {
                    this->addUnsupportedGrammarError(paramDecl->children[1]);
                    continue;
                }

                // parameter_declaration -> declaration_specifiers declarator
                AstNode* declarator = paramDecl->children[1];
                if (declarator->children.size() != 1) {
                    // 不支持指针。
                    this->addUnsupportedGrammarError(declarator->children[0]);
                    continue;
                }

                // direct_declarator
                AstNode* dirDecl = declarator->children[0];

                if (dirDecl->children.size() != 1) {
                    this->addUnsupportedGrammarError(paramDecl);
                    continue;
                }

                string& name = dirDecl->children[0]->token.content;
     

                functionParams.emplace_back();
                auto& param = functionParams.back();
                param.isPointer = false;
                param.isVaList = false;
                param.name = name;
                if (resContainer[0] == TokenKind::kw_int) {

                    param.type = ValueType::s32;
                } else {
                    param.type = ValueType::type_void;
                }

            }


        } while(false); else {

            this->addUnsupportedGrammarError(declaratorNode);
            return;

        }

        AstNode* identifierDirectDecl = directDeclarator->children[0];

        if (identifierDirectDecl->children[0]->symbolType != grammar::SymbolType::TERMINAL) {
            this->addUnsupportedGrammarError(identifierDirectDecl);
            return;
        }

        string& functionName = identifierDirectDecl->children[0]->token.content;

        cout << "func name: " << functionName << endl;

        // 生成函数信息。

        FunctionSymbol* funcSymbol = new FunctionSymbol;
        globalSymbolTable.functions[functionName] = funcSymbol;
        funcSymbol->params = functionParams;

        if (declarationSpecifierTokens[0] == TokenKind::kw_int) {
            funcSymbol->returnType = ValueType::s32;
        } else {
            funcSymbol->returnType = ValueType::type_void;
        }

        funcSymbol->isImported = false;
        funcSymbol->symbolType = SymbolType::functionDefine;
        funcSymbol->visibility = SymbolVisibility::global;

        this->currentFunction = funcSymbol;

        // 生成标签。
        instructionList.emplace_back();
        auto& irInstructionCode = instructionList.back();
        irInstructionCode.push_back("label");
        irInstructionCode.push_back(functionName);

        // 处理 compound statement

        // [ todo ]

        // 生成 ret 语句。
        // 这样做可能会导致重复生成 ret。后续删去多余的 ret 即可。
        instructionList.emplace_back();
        instructionList.back().push_back("ret");

        this->currentFunction = nullptr; // 不再绑定当前函数。

    } else {

        /*
            function_definition ->
                declaration_specifiers declarator 
                declaration_list compound_statement
        */

        this->addUnsupportedGrammarError(node);

    }

}

int tcir::IrGenerator::processDeclarationSpecifiers(
    AstNode* node, vector< TokenKind >& tokenListContainer
) {

    cout << "process declaration specifier" << endl;
    cout << "  " << node->symbol.name << endl;

    int prevErrors = this->errorList.size();

    if (node->children.size() != 1) {
        this->addUnsupportedGrammarError(node);
        cout << "  err 1" << endl;
        return 1;  
    }

    if (node->children[0]->symbolKind != SymbolKind::type_specifier) {
        this->addUnsupportedGrammarError(node);
        cout << "  err 2" << endl;
        return 1;
    }


    /*

        type_specifier  <- node.children
            : VOID      <- node.children[0].children[0]
            | CHAR
            | SHORT
            | INT
            | LONG
            | FLOAT
            | DOUBLE
            | SIGNED
            | UNSIGNED
            | struct_or_union_specifier
            | enum_specifier
            ;
    
    */

    AstNode* returnTypeSpecifier = node->children[0]->children[0];

    if (!returnTypeSpecifier->children.empty()) {
        this->addUnsupportedGrammarError(returnTypeSpecifier);
        cout << "  err 3" << endl;
        return 1;
    }

    if (returnTypeSpecifier->tokenKind == TokenKind::kw_int) {
        tokenListContainer.push_back(TokenKind::kw_int);
    } else if (returnTypeSpecifier->tokenKind == TokenKind::kw_void) {
        tokenListContainer.push_back(TokenKind::kw_void);
    } else {
        this->addUnsupportedGrammarError(returnTypeSpecifier);
        cout << "  err 4" << endl;
        return 1;
    }

    return this->errorList.size() - prevErrors;

}

