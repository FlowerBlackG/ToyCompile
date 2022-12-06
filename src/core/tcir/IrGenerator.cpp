/*
    TCIR 生成器。

    created on 2022.12.3
*/

#include <tc/core/tcir/IrGenerator.h>

using namespace std;
using namespace tc;

static tcir::IrInstructionCode __tcMakeIrInstruction(const string& tcirCode) {

    string code = tcirCode;

    tcir::IrInstructionCode ins;

    auto isBlank = [] (const char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    };

    while (code.length() > 0 && isBlank(code.back())) {
        code.pop_back();
    }

    if (code.length() == 0) {
        return ins;
    }

    string codeSegment;

    int currPos = 0;

    // code: "  xxxx   xxxx   xxxx"  <-- 结尾没有空格。
    while (currPos < code.length()) {
        char c = code[currPos];

        while (isBlank(c)) {
            c = code[++currPos];
        }

        codeSegment.clear();

        while (true) {
            codeSegment += c;

            if ((++currPos) >= code.length()) {
                break;
            }

            c = code[currPos];
            
            if (isBlank(c)) {
                break;
            }
        }

        ins.push_back(codeSegment);
    }

    return ins;
}

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

    this->nextLabelId = 1;
    this->nextVarId = 1;
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

        processVariableDeclaration(child, true);

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

            // 现在，所谓 "paramList" 只有一个孩子，并且它不是 paramList。
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

        this->processCompoundStatement(node->children[2]);

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


void tcir::IrGenerator::processCompoundStatement(AstNode* node) {

    if (node->children.size() == 2) {
        return; // compound_stmt -> { }
    }

    // compound_stmt -> { block_item_list }

    // 构建并启用符号表
    BlockSymbolTable* symbolTab = new BlockSymbolTable;
    symbolTab->parent = currentBlockSymbolTable ? currentBlockSymbolTable : symbolTab;
    currentBlockSymbolTable = symbolTab;
    symbolTab->descTable = &varDescTable;
    
    // 处理语句。
    this->processBlockItemList(node->children[1]);

    // 清除符号表
    if (symbolTab->parent == symbolTab) {
        currentBlockSymbolTable = nullptr;
    } else {
        currentBlockSymbolTable = symbolTab->parent;
    }

    delete symbolTab;
}

void tcir::IrGenerator::processBlockItemList(AstNode* node) {
    if (node->children.size() == 1) {
        this->processBlockItem(node->children[0]);
    } else {
        this->processBlockItemList(node->children[0]);
        this->processBlockItem(node->children[1]);
    }
}


void tcir::IrGenerator::processBlockItem(AstNode* node) {
    if (node->children[0]->symbolKind == SymbolKind::statement) {
        
        this->processStatement(node->children[0]);
        
    } else {
        
        this->processVariableDeclaration(node->children[0], false);

    }
}

void tcir::IrGenerator::processStatement(AstNode* node) {
    // todo.
}

void tcir::IrGenerator::processVariableDeclaration(AstNode* node, bool isInGlobalScope) {
    
    if (node->children.size() == 2) {
        // declaration -> declaration_specifiers ';'
        return;
    }

    // declaration -> declaration_specifiers init_declarator_list ';'
    vector<TokenKind> declSpecifierTokens;
    int resCode = processDeclarationSpecifiers(node->children[0], declSpecifierTokens);

    if (resCode > 0) {
        this->addUnsupportedGrammarError(node);
        return;
    }

    auto initDeclList = node->children[1];
    vector<AstNode*> initDecls;
    while (initDeclList->children.size() > 1) {
        // init_declarator_list -> init_declarator_list ',' init_declarator
        initDecls.push_back(initDeclList->children[2]);
        initDeclList = initDeclList->children[0];
    }

    initDecls.push_back(initDeclList->children[0]);
    for (auto it = initDecls.rbegin(); it != initDecls.rend(); it++) {
        this->processVariableInitDeclarator(*it, declSpecifierTokens, isInGlobalScope);
    }

}

void tcir::IrGenerator::processVariableInitDeclarator(
    AstNode* node, 
    vector<TokenKind>& declarationSpecifierTokens, 
    bool isInGlobalScope
) {
    ValueType valueType;
    // 目前仅支持 int
    if (declarationSpecifierTokens[0] == TokenKind::kw_int) {

        valueType = ValueType::s32;

    } else {
        errorList.emplace_back();
        errorList.back().astNode = node->children[0];
        errorList.back().msg = "unsupported value type. only int supported.";
        return;
    }

    // 先把 name 提取出来。
    AstNode* declarator = node->children[0];
    if (declarator->children.size() > 1) {
        // 指针。不支持。
        this->addUnsupportedGrammarError(declarator->children[0]);
        return;
    }

    AstNode* dirDecl = declarator->children[0];
    if (dirDecl->children.size() > 1) {
        // 仅支持 direct_decl -> IDENTIFIER
        this->addUnsupportedGrammarError(dirDecl);
        return;
    }

    string& idName = dirDecl->children[0]->token.content;

    VariableSymbol* symbol = new VariableSymbol;
    symbol->name = idName;
    symbol->bytes = ValueTypeUtils::getBytes(valueType);
    symbol->valueType = valueType;
    symbol->symbolType = SymbolType::variableDefine;

    if (isInGlobalScope) {
        symbol->visibility = SymbolVisibility::global;
    } else {
        symbol->visibility = SymbolVisibility::internal;
    }
        

    if (node->children.size() == 1) {
        // init_decl -> declarator

        /*
        
          例：

            int g;  <-- 这样。默认赋值为 0

            int main() {

                int x;  <-- 这样。未初始化。

                return 0;
            }
        
        */


        
        if (isInGlobalScope) {
            if (this->globalSymbolTable.variables.count(idName)) {
                this->warningList.emplace_back();
                this->warningList.back().astNode = node;
                this->warningList.back().msg = "symbol redefined: ";
                this->warningList.back().msg += idName;
            }

            if (this->globalSymbolTable.functions.count(idName)) {
                this->errorList.emplace_back();
                this->errorList.back().astNode = node;
                this->errorList.back().msg = "symbol defined as function: ";
                this->errorList.back().msg += idName;
                return;
            }


            this->globalSymbolTable.variables[idName] = symbol;
            
        } else {

            if (this->currentBlockSymbolTable->get(idName, false)) {
                this->errorList.emplace_back();
                auto& err = this->errorList.back();
                err.astNode = node;
                err.msg = "already defined: ";
                err.msg += idName;
                return;
            }


            symbol->id = this->nextVarId++;
            this->currentBlockSymbolTable->put(symbol);

        }

        return;
    }


    // init_decl -> declarator '=' initializer

    /*
      
      例：
    
        int g = 1;  <--

        int main() {

            int x = 2;  <--

            return 0;
        }

    */

    AstNode* initializer = node->children[2];

    if (initializer->children.size() > 1) {
        // { xxx }
        this->addUnsupportedGrammarError(initializer);
        return;
    }

    AstNode* assignmentExp = initializer->children[0];

    int prevErrCount = this->errorList.size();
    string expRes = this->processAssignmentExpression(assignmentExp, isInGlobalScope);

    if (errorList.size() - prevErrCount > 0) {
        return; // 遇到错误，不继续。
    }

    if (isInGlobalScope) {

        symbol->initValue = stoi(expRes);

        if (this->globalSymbolTable.variables.count(idName)) {
            this->warningList.emplace_back();
            this->warningList.back().astNode = node;
            this->warningList.back().msg = "symbol redefined: ";
            this->warningList.back().msg += idName;
        }

        if (this->globalSymbolTable.functions.count(idName)) {
            this->errorList.emplace_back();
            this->errorList.back().astNode = node;
            this->errorList.back().msg = "symbol defined as function: ";
            this->errorList.back().msg += idName;
            return;
        }

        this->globalSymbolTable.variables[idName] = symbol;

    } else {

        this->instructionList.emplace_back();
        auto& inst = this->instructionList.back();

        int varId = this->nextVarId++;
        symbol->id = varId;

        this->currentBlockSymbolTable->put(symbol);

        // 因为只考虑 int，这里直接等号就行。

        inst.push_back("mov");

        inst.push_back("val");
        inst.push_back(to_string(varId));

        inst.push_back("vreg");
        inst.push_back("0");
    }
}

string tcir::IrGenerator::processAssignmentExpression(
    AstNode* node, 
    bool isInGlobalScope
) {

    /*
    
        assignment_expression
            : conditional_expression
            | unary_expression assignment_operator assignment_expression
            ;
    
    */
    
    if (isInGlobalScope && node->children.size() > 1) {
        // int x = y += 2; <-- 暂不支持。
        this->addUnsupportedGrammarError(node);
        return "";
    }

    return processConditionalExpression(node->children[0], isInGlobalScope);

}

string tcir::IrGenerator::processConditionalExpression(
    AstNode* node, 
    bool isInGlobalScope
) {

    /*
    
        conditional_expression
            : logical_or_expression
            | logical_or_expression '?' expression ':' conditional_expression
            ;
    
    */

    int errCount = errorList.size();
    auto logiOrRes = processLogicalOrExpression(node->children[0], isInGlobalScope);

    if (errorList.size() - errCount) {
        return "";
    }
    
    if (node->children.size() == 1) {
        return logiOrRes;
    }

    // 三目表达式。
    if (stoll(logiOrRes)) {

        // expression

        return processExpression(node->children[2], isInGlobalScope);

    } else {

        // conditional expression

        return processConditionalExpression(node->children[4], isInGlobalScope);
    }

}

string tcir::IrGenerator::processLogicalOrExpression(
    AstNode* node, 
    bool isInGlobalScope
) {

    /*
    
        logical_or_expression
            : logical_and_expression
            | logical_or_expression OR_OP logical_and_expression
            ;
    
    */
    
    if (node->children.size() == 1) {
        return processLogicalAndExpression(node->children[0], isInGlobalScope);
    }

    int errCount = this->errorList.size();
    auto logiOrRes = processLogicalOrExpression(node->children[0], isInGlobalScope);
    if (errorList.size() - errCount) {
        return "";
    }

    if (isInGlobalScope) {

        long long res = stoll(logiOrRes);
        if (res) {
            return "1";
        }

        return processLogicalAndExpression(node->children[2], isInGlobalScope);

    } else {

        string resultLabel = ".logical_or_out_" + to_string(nextLabelId++);

        // 短路跳转。
        instructionList.push_back(__tcMakeIrInstruction("jne " + resultLabel));

        processLogicalAndExpression(node->children[2], isInGlobalScope);

        IrInstructionCode labelCode;
        labelCode.push_back("label");
        labelCode.push_back(resultLabel);
        instructionList.push_back(labelCode);

    }

}

string tcir::IrGenerator::processLogicalAndExpression(
    AstNode* node, 
    bool isInGlobalScope
) {


    /*
    
        logical_and_expression
            : inclusive_or_expression
            | logical_and_expression AND_OP inclusive_or_expression
            ;
    
    */
    
    if (node->children.size() == 1) {
        return processInclusiveOrExpression(node->children[0], isInGlobalScope);
    }

    int errCount = this->errorList.size();
    auto logiAndRes = processLogicalAndExpression(node->children[0], isInGlobalScope);

    if (errorList.size() - errCount) {
        return "";
    }

    if (isInGlobalScope) {
        long long res = stoll(logiAndRes);
        if (!res) {
            return "0";
        }

        return processInclusiveOrExpression(node->children[2], isInGlobalScope);

    } else {

        string resultLabel = ".logical_and_out_" + to_string(nextLabelId++);

        // 短路跳转。
        instructionList.push_back(__tcMakeIrInstruction("je " + resultLabel));

        processInclusiveOrExpression(node->children[2], isInGlobalScope);

        IrInstructionCode labelCode;
        labelCode.push_back("label");
        labelCode.push_back(resultLabel);
        instructionList.push_back(labelCode);

    }

}

string tcir::IrGenerator::processInclusiveOrExpression(
    AstNode* node, 
    bool isInGlobalScope
) {
    
    if (node->children.size() == 1) {
        return processExclusiveOrExpression(node->children[0], isInGlobalScope);
    } else {
        this->addUnsupportedGrammarError(node);
    }

}

string tcir::IrGenerator::processExclusiveOrExpression(
    AstNode* node, 
    bool isInGlobalScope
) {
    
    if (node->children.size() == 1) {
        return processAndExpression(node->children[0], isInGlobalScope);
    } else {
        this->addUnsupportedGrammarError(node);
    }

}


string tcir::IrGenerator::processAndExpression(
    AstNode* node, 
    bool isInGlobalScope
) {
    
    if (node->children.size() == 1) {
        return processEqualityExpression(node->children[0], isInGlobalScope);
    } else {

        this->addUnsupportedGrammarError(node);
    }

}


string tcir::IrGenerator::processEqualityExpression(AstNode* node, bool isInGlobalScope) {


    /*
    
        equality_expression
            : relational_expression
            | equality_expression EQ_OP relational_expression
            | equality_expression NE_OP relational_expression
            ;

    */

    if (node->children.size() == 1) {
        return this->processRelationalExpression(node->children[0], isInGlobalScope);
    }

    int errCount = this->errorList.size();

    auto eqExpResult = processEqualityExpression(node->children[0], isInGlobalScope);

    auto opToken = node->children[1]->tokenKind;

    if (errorList.size() - errCount) {
        return "";
    }

    if (isInGlobalScope) {
        auto relationExpRes = processRelationalExpression(
            node->children[2], isInGlobalScope
        );

        if (errorList.size() - errCount) {
            return "";
        }

        long long res = stoll(eqExpResult);
        if (opToken == TokenKind::equalequal) {
            res = res == stoll(relationExpRes);
        } else { // not eq
            res = res != stoll(relationExpRes);
        }

        return to_string(res);

    } else {

        this->instructionList.push_back(__tcMakeIrInstruction("push 4 vreg 0"));

        auto relationExpRes = processRelationalExpression(
            node->children[2], isInGlobalScope
        );

        if (errorList.size() - errCount) {
            return "";
        }

        this->instructionList.push_back(__tcMakeIrInstruction("pop 4 vreg 1"));

        auto ins = __tcMakeIrInstruction("cmp vreg 1 vreg 0");
        

        if (opToken == TokenKind::equalequal) {

            ins.push_back("eq");

        } else {
            ins.push_back("ne");
        }

        this->instructionList.push_back(ins);

        return "";

    }



}

string tcir::IrGenerator::processRelationalExpression(AstNode* node, bool isInGlobalScope) {

    /*
    
        relational_expression
            : shift_expression
            | relational_expression '<' shift_expression
            | relational_expression '>' shift_expression
            | relational_expression LE_OP shift_expression
            | relational_expression GE_OP shift_expression
            ;

    */


    if (node->children.size() == 1) {
        return this->processShiftExpression(node->children[0], isInGlobalScope);
    }

    int errCount = errorList.size();


    auto relationalExpResult = processRelationalExpression(
        node->children[0], isInGlobalScope
    );

    if (errorList.size() - errCount) {
        return "";
    }


    auto opToken = node->children[1]->tokenKind;

    if (isInGlobalScope) {
        auto shiftExpRes = this->processShiftExpression(node->children[2], isInGlobalScope);
        
        if (errorList.size() - errCount) {
            return "";
        }

        long long result = stoll(relationalExpResult);
        long long shiftRes = stoll(shiftExpRes);

        if (opToken == TokenKind::less) {
            result = !!(result < shiftRes);
        } else if (opToken == TokenKind::greater) {
            result = !!(result > shiftRes);
        } else if (opToken == TokenKind::lessequal) {
            result = !!(result <= shiftRes);
        } else {
            result = !!(result >= shiftRes);
        }

        return to_string(result);

    } else {

        this->instructionList.push_back(__tcMakeIrInstruction("push 4 vreg 0"));

        auto shiftExpRes = this->processShiftExpression(node->children[2], isInGlobalScope);

        if (errorList.size() - errCount) {
            return "";
        }

        this->instructionList.push_back(__tcMakeIrInstruction("pop 4 vreg 1"));

        IrInstructionCode ins;

        ins.push_back("cmp");
        ins.push_back("vreg");
        ins.push_back("1");
        ins.push_back("vreg");
        ins.push_back("0");
        
        if (opToken == TokenKind::less) {
            ins.push_back("l");
        } else if (opToken == TokenKind::greater) {
            ins.push_back("g");
        } else if (opToken == TokenKind::lessequal) {
            ins.push_back("le");
        } else {
            ins.push_back("ge");
        }

        this->instructionList.push_back(ins);

        return "";

    }

}


string tcir::IrGenerator::processShiftExpression(AstNode* node, bool isInGlobalScope) {

    /*
    
        shift_expression
            : additive_expression
            | shift_expression LEFT_OP additive_expression
            | shift_expression RIGHT_OP additive_expression
            ;
    
    */

    if (node->children.size() > 1) {
        this->addUnsupportedGrammarError(node->children[1]);
        return "";
    }

    return this->processAdditiveExpression(node->children[0], isInGlobalScope);

}

string tcir::IrGenerator::processAdditiveExpression(AstNode* node, bool isInGlobalScope) {

    /*
    
        additive_expression
            : multiplicative_expression
            | additive_expression '+' multiplicative_expression
            | additive_expression '-' multiplicative_expression
            ;
    
    */

    int errCount = errorList.size();

    if (node->children.size() == 1) {
        return processMultiplicativeExpression(node->children[0], isInGlobalScope);
    }

    auto addResult = processAdditiveExpression(
        node->children[0], isInGlobalScope
    );


    if (errorList.size() - errCount) {
        return "";
    }

    auto opToken = node->children[1]->tokenKind;
    if (isInGlobalScope) {

        
        auto multiplicationResult = processMultiplicativeExpression(
            node->children[2], isInGlobalScope
        );

        if (errorList.size() - errCount) {
            return "";
        }
    
        long long result = stoll(addResult);
        long long mulRes = stoll(multiplicationResult);
        if (opToken == TokenKind::plus) {
            result += mulRes;
        } else {
            result -= mulRes;
        }

        return to_string(result);
    
    } else {

        

        this->instructionList.push_back(__tcMakeIrInstruction("push 4 vreg 0"));

        auto multiplicationResult = processMultiplicativeExpression(
            node->children[2], isInGlobalScope
        );

        if (errorList.size() - errCount) {
            return "";
        }

        this->instructionList.push_back(__tcMakeIrInstruction("pop 4 vreg 1"));

        // add/sub vreg 0 vreg 1
        IrInstructionCode ins;
        if (opToken == TokenKind::plus) {
            ins.push_back("add");
        } else {
            ins.push_back("sub");
        }

        ins.push_back("vreg");
        ins.push_back("1");
        ins.push_back("vreg");
        ins.push_back("0");
        this->instructionList.push_back(ins);

        // 计算结果位于 vreg 1. 我们希望把它存到 vreg 0 内。
        this->instructionList.push_back(__tcMakeIrInstruction("xchg vreg 0 vreg 1"));

        return "";

    }

}

string tcir::IrGenerator::processMultiplicativeExpression(AstNode* node, bool isInGlobalScope) {

    /*

        multiplicative_expression
            : cast_expression
            | multiplicative_expression '*' cast_expression
            | multiplicative_expression '/' cast_expression
            | multiplicative_expression '%' cast_expression
            ;

    */

    if (node->children.size() == 1) {
        return processCastExpression(node->children[0], isInGlobalScope);
    }

    this->addUnsupportedGrammarError(node->children[1]); // 不支持乘法除法取模。
    return "";

}

string tcir::IrGenerator::processCastExpression(AstNode* node, bool isInGlobalScope) {

    /*
    
        cast_expression
            : unary_expression
            | '(' type_name ')' cast_expression
            ;

    */

    // 暂不支持真的转换。
    if (node->children.size() > 1) {
        return processCastExpression(node->children[3], isInGlobalScope);
    }

    return processUnaryExpression(node->children[0], isInGlobalScope);

}

string tcir::IrGenerator::processExpression(AstNode* node, bool isInGlobalScope) {

    /*
    
        expression
            : assignment_expression
            | expression ',' assignment_expression
            ;
    
    */

    if (node->children.size() == 1) {
        return processAssignmentExpression(node->children[0]);
    }

    int errCount = errorList.size();
    processExpression(node->children[0], isInGlobalScope);

    if (errorList.size() - errCount) {
        return "";
    }

    auto assignmentExpRes = processAssignmentExpression(node->children[2], isInGlobalScope);

    return assignmentExpRes;

}

string tcir::IrGenerator::processUnaryExpression(AstNode* node, bool isInGlobalScope) {
// todo
}


string tcir::IrGenerator::processPostfixExpression(AstNode* node, bool isInGlobalScope) {
// todo
}

string tcir::IrGenerator::processPrimaryExpression(AstNode* node, bool isInGlobalScope) {
    
    /*
    
        primary_expression
            : IDENTIFIER
            | CONSTANT
            | STRING_LITERAL
            | '(' expression ')'
            ;
    
    */

    if (node->children.size() == 3) {
        return processExpression(node->children[1], isInGlobalScope);
    }

    auto tokenKind = node->children[0]->tokenKind;
    auto& content = node->children[0]->token.content;

    if (tokenKind == TokenKind::string_literal) {
        // 暂不支持字符串。后续应该考虑支持。
        this->addUnsupportedGrammarError(node->children[0]);
    } else if (tokenKind == TokenKind::identifier) {

        if (isInGlobalScope) {
            errorList.emplace_back();
            auto& err = errorList.back();
            err.astNode = node->children[0];
            err.msg = "cannot use variable to init value in global scope. (";
            err.msg += content;
            err.msg += ")";

            return "";

        } else {

            
            IrInstructionCode ir;
            ir.push_back("mov");
            ir.push_back("vreg");
            ir.push_back("0");

            // 寻找这个符号的含义。

            // 先从块符号表找。
            auto symbolFromTable = currentBlockSymbolTable->get(content, true);

            if (symbolFromTable && symbolFromTable->valueType != ValueType::s32) {
                this->errorList.emplace_back();
                auto& err = errorList.back();
                err.astNode = node->children[0];
                err.msg += "only support int32.";
                return "";
            }

            if (symbolFromTable) {
                ir.push_back("val");
                ir.push_back(to_string(symbolFromTable->id));
                instructionList.push_back(ir);
                return "";
            }

            // 从函数参数表找。
            auto symFromFuncParams = currentFunction->findParamSymbol(content);

            if (symFromFuncParams && symFromFuncParams->type != ValueType::s32) {
                
                this->errorList.emplace_back();
                auto& err = errorList.back();
                err.astNode = node->children[0];
                err.msg += "only support int32.";
                return "";
            }

            if (symFromFuncParams) {

                ir.push_back("fval");
                ir.push_back(content);
                instructionList.push_back(ir);
                return "";

            }

            // 最后，从全局变量找。
            auto symFromGlobalVar = globalSymbolTable.getVariable(content);

            if (symFromGlobalVar && symFromGlobalVar->valueType != ValueType::s32) {
                this->errorList.emplace_back();
                auto& err = errorList.back();
                err.astNode = node->children[0];
                err.msg += "only support int32.";
                return "";
            }

            if (symFromGlobalVar) {
                ir.push_back("val");
                ir.push_back(content);
                instructionList.push_back(ir);
                return "";

            }

            errorList.emplace_back();
            auto& err = errorList.back();
            err.astNode = node->children[0];
            err.msg = "symbol not found: ";
            err.msg += content;

            return "";
        }

    } else {
        // constant
        // 暂不支持浮点。

        long long value = stoll(content);

        if (isInGlobalScope) {
            return content;
        } else {
            instructionList.push_back(__tcMakeIrInstruction("mov vreg 0 imm " + content));
        }
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

