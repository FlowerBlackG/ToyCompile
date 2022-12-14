/*
    TCIR 生成器。

    created on 2022.12.3
*/

#include <tc/core/tcir/IrGenerator.h>
#include <tc/utils/ConsoleColorPad.h>

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

void tcir::IrGenerator::dump(ostream& out, bool withColor) {

    auto setColor = [withColor] (int red = -1, int green = 0, int blue = 0) {
        if (withColor) {
            if (red != -1) {
                ConsoleColorPad::setColor(red, green, blue);
            } else {
                ConsoleColorPad::setColor();
            }
        }
    };

    /* 符号关联。 */
    
    setColor(0xed, 0x5a, 0x65);
    out << "@ begin of extlink" << endl;
    // 函数导出。
    for (auto& fun : this->globalSymbolTable.functions) {
        if (fun.second->visibility != SymbolVisibility::global) {
            continue;
        }

        out << "export " << fun.first << " fun" << endl;
    }

    // 变量导出。
    for (auto& var : this->globalSymbolTable.variables) {
        out << "export " << var.first << " var" << endl;
    }

    // 暂不支持导入。
    out << "@ end of extlink" << endl;
    out << endl;

    /* 全局数据。 */
    setColor(0x7e, 0x16, 0x71);
    out << "@ begin of static-data" << endl;
    for (auto& var : this->globalSymbolTable.variables) {
        out << "int var " << var.first;
        out << " " << ValueTypeUtils::getName(var.second->valueType);
        out << " " << var.second->initValue << endl;
    }
    out << "@ end of static-data" << endl;
    out << endl;

    /* 全文符号表。 */
    setColor(0x11, 0x77, 0xb0);
    out << "@ begin of global-symtab" << endl;

    // 函数
    for (auto& fun : this->globalSymbolTable.functions) {
        out << "fun ";
        if (fun.second->visibility == SymbolVisibility::global) {
            out << "visible ";
        } else {
            out << "internal ";
        }

        out << fun.first << " " << fun.second->params.size() << " ";
        out << ValueTypeUtils::getName(fun.second->returnType) << endl;

        for (auto& param : fun.second->params) {
            out << "  ";
            out << ValueTypeUtils::getName(param.valueType) << " ";
            out << (param.isPointer ? "ptr" : "value") << " ";
            out << param.name << endl;
            // 暂不支持变长参数。
        }
    }

    // 变量（临时）
    for (auto& var : this->varDescTable.symbolMap) {
        out << "var " << var.first << " ";
        out << var.second->name << " ";
        out << ValueTypeUtils::getName(var.second->valueType);
        out << " " << ValueTypeUtils::getBytes(var.second->valueType);
        out << endl;
    }

    out << "@ end of global-symtab" << endl;
    out << endl;

    // 块符号表。
    
    setColor(0x43, 0xb2, 0x44);
    out << "@ begin of block-symtab" << endl;
    out << blockSymbolTableIrDumps.str() << endl;
    out << "@ end of block-symtab" << endl;
    out << endl;

    /* 指令。 */
    setColor(0xfc, 0xa1, 0x06);
    out << "@ begin of instructions" << endl;
    for (auto& insCode : this->instructionList) {
        for (auto& codeSegment : insCode) {
            out << codeSegment << " ";
        }
        out << endl;
    }
    out << "@ end of instructions" << endl;

    setColor();
    
    out << endl;

}

void tcir::IrGenerator::clear() {
    this->varDescTable.clear();
    this->errorList.clear();
    this->globalSymbolTable.clear();
    this->instructionList.clear();
    this->blockSymbolTableIrDumps.str(string());

    if (this->currentBlockSymbolTable) {
        delete this->currentBlockSymbolTable;
        this->currentBlockSymbolTable = nullptr;
    }

    this->nextLabelId = 1;
    this->nextVarId = 1;
    this->nextBlockSymTabId = 1;
}

void tcir::IrGenerator::addUnsupportedGrammarError(AstNode* node) {
    
    auto& err = errorList.emplace_back();

    AstNode* firstToken = node;
    while (firstToken->symbolType == grammar::SymbolType::NON_TERMINAL) {
        firstToken = firstToken->children[0];
    }

    err.astNode = node;

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
                
                auto& param = functionParams.emplace_back();
                
                param.symbolType = SymbolType::functionParam;
                param.isPointer = false;
                param.isVaList = false;
                param.name = name;
                if (resContainer[0] == TokenKind::kw_int) {

                    param.valueType = ValueType::s32;
                } else {
                    param.valueType = ValueType::type_void;
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
    symbolTab->id = nextBlockSymTabId++;
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

    symbolTab->dump(this->blockSymbolTableIrDumps);
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


    /*
    
        statement
            : labeled_statement
            | compound_statement
            | expression_statement
            | selection_statement
            | iteration_statement
            | jump_statement
            ;
    
    */

    switch (node->children[0]->symbolKind) {

        case SymbolKind::labeled_statement: {
            this->addUnsupportedGrammarError(node->children[0]);

            break;
        }
        
        case SymbolKind::compound_statement: {

            this->processCompoundStatement(node->children[0]);

            break;
        }
        
        case SymbolKind::expression_statement: {

            /*
            
                expression_statement
                    : ';'
                    | expression ';'
                    ;
            
            */

            processExpressionStatement(node->children[0]);

            break;
        }
        
        case SymbolKind::selection_statement: {
            
            this->processSelectionStatement(node->children[0]);

            break;
        }
        
        case SymbolKind::iteration_statement: {

            this->processIterationStatement(node->children[0]);

            break;
        }

        case SymbolKind::jump_statement: {
            this->processJumpStatement(node->children[0]);

            break;
        }

        default: {

            auto& err = errorList.emplace_back();
            err.astNode = node;
            err.msg = "internal error: code f8218f11";

            break;
        }
    }


}

void tcir::IrGenerator::processSelectionStatement(AstNode* node) {

    if (node->children[0]->tokenKind == TokenKind::kw_switch) {
        this->addUnsupportedGrammarError(node->children[0]);
        return; // 暂不支持 switch case 语句。
    }

    string endLabel = ".if_end_" + to_string(nextLabelId++);

    processExpression(node->children[2], false);

    bool hasElseStmt = node->children.size() == 7;

    string elseLabel;

    if (hasElseStmt) {

        elseLabel = ".if_else_" + to_string(nextLabelId++);

        instructionList.push_back(__tcMakeIrInstruction(
            "je " + elseLabel
        ));

    } else {

        instructionList.push_back(__tcMakeIrInstruction(
            "je " + endLabel
        ));
    
    }

    processStatement(node->children[4]);

    if (hasElseStmt) {

        instructionList.push_back(__tcMakeIrInstruction(
            "jmp " + endLabel
        ));

        instructionList.push_back(__tcMakeIrInstruction(
            "label " + elseLabel
        ));

        processStatement(node->children[6]);
        
    } 

    instructionList.push_back(__tcMakeIrInstruction(
        "label " + endLabel
    ));
    

}

void tcir::IrGenerator::processIterationStatement(AstNode* node) {

    // iteration_statement ->

    if (node->children[0]->tokenKind == TokenKind::kw_while) {

        // WHILE '(' expression ')' statement

        processIterationStatementWhileLoop(node->children[2], node->children[4]);
        
    } else if (node->children[0]->tokenKind == TokenKind::kw_do) {

        // DO statement WHILE '(' expression ')' ';'

        processIterationStatementDoWhile(node->children[1], node->children[4]);
        
    } else if (node->children.size() == 6) {

        // FOR '(' expression_statement expression_statement ')' statement
        // FOR '(' declaration          expression_statement ')' statement

        // for (int x = 1; x < 10;) { ... }

        this->processIterationStatementForLoop(
            node->children[2], node->children[3], 
            nullptr, node->children.back()
        );

    } else {

        // FOR '(' declaration          expression_statement expression ')' statement
        // FOR '(' expression_statement expression_statement expression ')' statement
    
        this->processIterationStatementForLoop(
            node->children[2], node->children[3], 
            node->children[4], node->children.back()
        );
    
    }


}

void tcir::IrGenerator::processIterationStatementDoWhile(
    AstNode* statement, 
    AstNode* expression
) {

    /*

        |c|

            do 
                statement
            while (expression);

        |ir|

            stmt:
                statement

            exp:  <- continue target
                expression
                
                je end
                j stmt
            
            end:  <- break target

    */

    string&& labelIdStr = to_string(nextLabelId++);

    string&& stmtLabel = ".do_while_stmt_" + labelIdStr;
    string&& expLabel = ".do_while_exp_" + labelIdStr;
    string&& endLabel = ".do_while_end_" + labelIdStr;

    this->continueStmtTargets.push_back(expLabel);
    this->breakStmtTargets.push_back(endLabel);

    instructionList.push_back(__tcMakeIrInstruction("label " + stmtLabel));

    processStatement(statement);
    
    instructionList.push_back(__tcMakeIrInstruction("label " + expLabel));
    
    processExpression(expression, false);
    instructionList.push_back(__tcMakeIrInstruction("je " + endLabel));
    instructionList.push_back(__tcMakeIrInstruction("j " + stmtLabel));

    instructionList.push_back(__tcMakeIrInstruction("label " + endLabel));


    this->continueStmtTargets.pop_back();
    this->breakStmtTargets.pop_back();

}

void tcir::IrGenerator::processIterationStatementWhileLoop(
    AstNode* expression, 
    AstNode* statement
) {

    /*
    
        |c|

            while (expression)
                statement
    

        |ir|

            exp:  <- continue target
                expression
                je end

            stmt:
                statement
                jmp exp


            end:  <- break target

    */

    string&& labelIdStr = to_string(nextLabelId++);

    string&& stmtLabel = ".while_loop_stmt_" + labelIdStr;
    string&& expLabel = ".while_loop_exp_" + labelIdStr;
    string&& endLabel = ".while_loop_end_" + labelIdStr;

    this->continueStmtTargets.push_back(expLabel);
    this->breakStmtTargets.push_back(endLabel);

    instructionList.push_back(__tcMakeIrInstruction("label " + expLabel));
    processExpression(expression, false);
    instructionList.push_back(__tcMakeIrInstruction("je " + endLabel));
    instructionList.push_back(__tcMakeIrInstruction("label " + stmtLabel));
    processStatement(statement);
    instructionList.push_back(__tcMakeIrInstruction("jmp " + expLabel));
    instructionList.push_back(__tcMakeIrInstruction("label " + endLabel));
    

    this->continueStmtTargets.pop_back();
    this->breakStmtTargets.pop_back();

}


void tcir::IrGenerator::processIterationStatementForLoop(
    AstNode* expStmtOrDeclaration, 
    AstNode* expStmt, 
    AstNode* expression /* nullable */,
    AstNode* statement 
) {

    if (expStmtOrDeclaration->symbolKind == SymbolKind::declaration) {
        this->addUnsupportedGrammarError(expStmtOrDeclaration);
        // 暂不支持 for (int x = 0; ; ) {} 这种形式。循环变量要求在外部定义。
        return;
    }

    /*
    
        |c|
            for (expStmtOrDecl (;) expStmt (;) expression)
                statement

        |ir|

                expStmtOrDecl
            
            estmt:

                expStmt
                je end

                statement

            exp:  <- continue target
                expression
                jmp estmt

            end:  <- break target
    
    */

    auto pushIr = [this] (const string& ir) {
        this->instructionList.push_back(__tcMakeIrInstruction(ir));
    };

    string&& labelIdStr = to_string(nextLabelId++);
    string&& estmtLabel = ".for_loop_estmt_" + labelIdStr;
    string&& expLabel = ".for_loop_exp_" + labelIdStr;
    string&& endLabel = ".for_loop_end_" + labelIdStr;

    this->continueStmtTargets.push_back(expLabel);
    this->breakStmtTargets.push_back(endLabel);

    // expStmtOrDecl
    if (expStmtOrDeclaration->symbolKind == SymbolKind::declaration) {
        this->addUnsupportedGrammarError(expStmtOrDeclaration);
        // 暂不支持 for (int x = 0; ; ) {} 这种形式。循环变量要求在外部定义。
        return;
    } else {

        processExpressionStatement(expStmtOrDeclaration);

    }

    pushIr("label " + estmtLabel);
    this->processExpressionStatement(expStmt);
    pushIr("je " + endLabel);

    processStatement(statement);

    pushIr("label " + expLabel);
    processExpression(expression, false);
    pushIr("jmp " + estmtLabel);
    pushIr("label " + endLabel);

    this->continueStmtTargets.pop_back();
    this->breakStmtTargets.pop_back();

}

void tcir::IrGenerator::processJumpStatement(AstNode* node) {

    switch (node->children[0]->tokenKind) {

        case TokenKind::kw_goto: {
            
            auto& err = errorList.emplace_back();
            err.astNode = node->children[0];
            err.msg = "\"goto\" is not currently supported.";

            break;
        }

        case TokenKind::kw_continue: {

            if (continueStmtTargets.empty()) {
                auto& err = errorList.emplace_back();
                err.astNode = node->children[0];
                err.msg = "nowhere to skip for \"continue\".";
               
            } else {

                instructionList.push_back(__tcMakeIrInstruction(
                    "jmp " + continueStmtTargets.back()
                ));
            }

            break;
        }

        case TokenKind::kw_break: {

            if (breakStmtTargets.empty()) {
                auto& err = errorList.emplace_back();
                err.astNode = node->children[0];
                err.msg = "nowhere to skip for \"break\".";
                
            } else {

                instructionList.push_back(__tcMakeIrInstruction(
                    "jmp " + breakStmtTargets.back()
                ));
            }

            break;
        }

        case TokenKind::kw_return: {

            if (node->children.size() == 3) {
                processExpression(node->children[1], false);
            }

            instructionList.push_back(__tcMakeIrInstruction("ret"));

            break;
        }

        default: {

            break;
        }
    }

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

        
    } else {

        if (this->currentBlockSymbolTable->get(idName, false)) {
            auto& err = this->errorList.emplace_back();
            err.astNode = node;
            err.msg = "already defined: ";
            err.msg += idName;


            return;
        }


        symbol->id = this->nextVarId++;
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

            this->globalSymbolTable.variables[idName] = symbol;
        } else {

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

    // 注册到符号表。在 assignment exp 之后注册，防止表达式内直接调用自己。
    if (isInGlobalScope) {

        this->globalSymbolTable.variables[idName] = symbol;
    } else {

        this->currentBlockSymbolTable->put(symbol);
    }

    if (isInGlobalScope) {

        this->globalSymbolTable.variables[idName]->initValue = stoi(expRes);

    } else {

        auto& inst = this->instructionList.emplace_back();

        int varId = symbol->id;

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

    if (node->children.size() == 1) {

        return processConditionalExpression(node->children[0], isInGlobalScope);
    }
   
    
    if (isInGlobalScope) {
        this->addUnsupportedGrammarError(node);

        return "";
    }
        

    int errCount = errorList.size();

    this->processUnaryExpression(node->children[0], isInGlobalScope);
    // 执行完上方语句，directSymbol 会被设置。

    if (errorList.size() - errCount) {
        return "";
    }

    if (!directResultSymbol) {
        auto& err = errorList.emplace_back();

        err.astNode = node;
        err.msg = "cannot find symbol.";

        return "";
    }

    auto dirSymbol = directResultSymbol;
    string valueName = this->symbolToIrValueCode(dirSymbol);

    TokenKind op = node->children[1]->children[0]->tokenKind;

    this->processAssignmentExpression(node->children[2], isInGlobalScope);
    if (errCount - errorList.size()) {
        return "";
    }

    switch (op) {
        case TokenKind::equal: {

            instructionList.push_back(__tcMakeIrInstruction(
                "mov " + valueName + " vreg 0"
            ));

            break;
        }

        case TokenKind::plusequal: {
            this->addUnsupportedGrammarError(node->children[1]->children[0]);
            // 暂不支持 +=
            break;
        }
        
        case TokenKind::minusequal: {
            this->addUnsupportedGrammarError(node->children[1]->children[0]);
            // 暂不支持 -=
            break;
        }

        default: {
            this->addUnsupportedGrammarError(node->children[1]->children[0]);
            return "";
        }
    }

    return "";

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
    if (isInGlobalScope) {
        if (stoll(logiOrRes)) {

            // expression

            return processExpression(node->children[2], isInGlobalScope);

        } else {

            // conditional expression

            return processConditionalExpression(node->children[4], isInGlobalScope);
        }

    } else {

        /*
        
                logocal_or_exp
                je .false
                expression
                jmp .exit
            .false:
                conditional_exp
            .exit:
        
        */

        auto labelId = this->nextLabelId++;
        auto&& exitLabel = ".con_exit_" + to_string(labelId);
        auto&& falseLabel = ".con_false_" + to_string(labelId);

        this->instructionList.push_back(__tcMakeIrInstruction(
            "je " + falseLabel
        ));

        this->processExpression(node->children[2], isInGlobalScope);

        this->instructionList.push_back(__tcMakeIrInstruction(
            "jmp " + exitLabel
        ));

        this->instructionList.push_back(__tcMakeIrInstruction(
            "label " + falseLabel
        ));

        this->processConditionalExpression(node->children[4], isInGlobalScope);

        this->instructionList.push_back(__tcMakeIrInstruction(
            "label " + exitLabel
        ));

        return "";

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

        return "";

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

        return "";

    }

}

string tcir::IrGenerator::processInclusiveOrExpression(
    AstNode* node, 
    bool isInGlobalScope
) {

    /*
    
        inclusive_or_expression
            : exclusive_or_expression
            | inclusive_or_expression '|' exclusive_or_expression
            ;
    
    */
    
    if (node->children.size() == 1) {
        return processExclusiveOrExpression(node->children[0], isInGlobalScope);
    } 
    
    if (isInGlobalScope) {

        auto res1 = processInclusiveOrExpression(node->children[0], isInGlobalScope);
        auto res2 = processExclusiveOrExpression(node->children[2], isInGlobalScope);

        return to_string(stoll(res1) | stoll(res2));

    } else {
        this->addUnsupportedGrammarError(node);
        return "";
    }

}

string tcir::IrGenerator::processExclusiveOrExpression(
    AstNode* node, 
    bool isInGlobalScope
) {

    /*
    
        exclusive_or_expression
            : and_expression
            | exclusive_or_expression '^' and_expression
            ;
    
    */

    if (node->children.size() == 1) {
        return processAndExpression(node->children[0], isInGlobalScope);
    } 
    
    if (isInGlobalScope) {

        auto&& res1 = processExclusiveOrExpression(node->children[0], isInGlobalScope);
        auto&& res2 = processAndExpression(node->children[2], isInGlobalScope);

        return to_string(stoll(res1) ^ stoll(res2));

    } else {
        this->addUnsupportedGrammarError(node);
        return "";
    }

}


string tcir::IrGenerator::processAndExpression(
    AstNode* node, 
    bool isInGlobalScope
) {

    /*
    
        and_expression
            : equality_expression
            | and_expression '&' equality_expression
            ;
    
    */
    
    if (node->children.size() == 1) {
        return processEqualityExpression(node->children[0], isInGlobalScope);
    } 
    
    if (isInGlobalScope) {

        auto&& res1 = processAndExpression(node->children[0], isInGlobalScope);
        auto&& res2 = processEqualityExpression(node->children[2], isInGlobalScope);

        return to_string(stoll(res1) & stoll(res2));

    } else {

        this->addUnsupportedGrammarError(node);
        return "";
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

void tcir::IrGenerator::processExpressionStatement(AstNode* node) {

    if (node->children.size() > 1) {
        processExpression(node->children[0], false);
    }

    
    directResultSymbol = nullptr; // 防止该值被设置。

}

string tcir::IrGenerator::processExpression(AstNode* node, bool isInGlobalScope) {

    /*
    
        expression
            : assignment_expression
            | expression ',' assignment_expression
            ;
    
    */


    if (node->children.size() == 1) {
        return processAssignmentExpression(node->children[0], isInGlobalScope);
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


    /*
        
        unary_expression
            : postfix_expression
            | INC_OP unary_expression
            | DEC_OP unary_expression
            | unary_operator cast_expression
            | SIZEOF unary_expression
            | SIZEOF '(' type_name ')'
            ;
    
    */

    if (node->children.size() == 1) {
        return processPostfixExpression(node->children[0], isInGlobalScope);
    }

    if (node->children[0]->symbolType == grammar::SymbolType::TERMINAL) {
        if (node->children[0]->tokenKind == TokenKind::kw_sizeof) {
            // 不支持 sizeof
            this->addUnsupportedGrammarError(node->children[0]);
            return "";
        }

        if (isInGlobalScope) {

            auto& err = errorList.emplace_back();
            err.astNode = node;
            err.msg = "cannot use ++/-- in global scope.";

            return "";
        }

        if (!directResultSymbol) {
            auto& err = errorList.emplace_back();
            err.astNode = node;
            err.msg = "cannot use ++/-- on constant value.";

            return "";
        }

        string valueCode = this->symbolToIrValueCode(directResultSymbol);

        if (node->children[0]->tokenKind == TokenKind::plusplus) {
            
            // ++i
            this->instructionList.push_back(__tcMakeIrInstruction(
                "add " + valueCode + " imm 1"
            ));

        } else {
        
            // --i
            this->instructionList.push_back(__tcMakeIrInstruction(
                "sub " + valueCode + " imm 1"
            ));
        
        }

        this->instructionList.push_back(__tcMakeIrInstruction(
            "mov vreg 0 " + valueCode
        ));

    }

    /*

      剩下未处理：
        
        unary_expression
            : unary_operator cast_expression
            ;
    
    */

    // 摆烂了，不处理了！
    this->addUnsupportedGrammarError(node->children[0]);
    return "";
    // todo: 不难处理。有空可以填坑。
}


string tcir::IrGenerator::processPostfixExpression(AstNode* node, bool isInGlobalScope) {

    /*
        postfix_expression
            : primary_expression
            | postfix_expression '[' expression ']'
            | postfix_expression '(' ')'
            | postfix_expression '(' argument_expression_list ')'
            | postfix_expression '.' IDENTIFIER
            | postfix_expression PTR_OP IDENTIFIER
            | postfix_expression INC_OP
            | postfix_expression DEC_OP
            | '(' type_name ')' '{' initializer_list '}'
            | '(' type_name ')' '{' initializer_list ',' '}'
            ;
    
    */


    int errCount = errorList.size();

    if (node->children.size() == 1) {
        
        return processPrimaryExpression(node->children[0], isInGlobalScope);

    } else if (node->children.size() == 2) {

        TokenKind op = node->children[1]->tokenKind;

        auto postfixExpRes = processPostfixExpression(node->children[0], isInGlobalScope);

        if (resultValueType != ValueType::s32) {

            auto& err = errorList.emplace_back();
            err.astNode = node->children[0];
            err.msg += "cannot assign ++/-- on non int32 value.";
            return "";

        }

        if (isInGlobalScope) {
            auto& err = errorList.emplace_back();
            err.astNode = node->children[0];
            err.msg += "cannot assign ++/-- in global scope.";
            return "";
        }

        if (!directResultSymbol) {
            
            auto& err = errorList.emplace_back();
            err.astNode = node->children[0];
            err.msg += "cannot assign ++/-- to constants.";
            return "";
        }

        string symbolCode = this->symbolToIrValueCode(directResultSymbol);

        instructionList.push_back(__tcMakeIrInstruction(
            "mov vreg 0 " + symbolCode
        ));

        if (op == TokenKind::plusplus) {
            // postfix_exp INC_OP

            instructionList.push_back(__tcMakeIrInstruction(
                "add " + symbolCode + " imm 1"
            ));

        } else {
            // postfix_exp DEC_OP

            instructionList.push_back(__tcMakeIrInstruction(
                "sub " + symbolCode + " imm 1"
            ));
        }

        return "";

    }

    if (node->children[0]->symbolType == grammar::SymbolType::TERMINAL) {
        this->addUnsupportedGrammarError(node);
        // 不支持 '(' type_name ')' '{' initializer_list ',' '}'

        return "";
    }

    if (node->children[1]->tokenKind == TokenKind::l_square // []
        || node->children[1]->tokenKind == TokenKind::period // x . y
        || node->children[1]->tokenKind == TokenKind::arrow // x -> y
    ) {
        this->addUnsupportedGrammarError(node->children[1]);
        return "";
    }



    /*

      剩余未处理的：

        postfix_expression
            | postfix_expression '(' ')'
            | postfix_expression '(' argument_expression_list ')'
            ;
        
        函数调用。
    
    */

    // ================================================
    //
    //
    //
    //
    //
    //   // todo: 函数调用是课设必做项目。这里先搁置。
    //
    //
    //
    //
    //
    // - - - - - - - - - - - - - - - - - - - - - - - - 
    //
    //
    //
    //
        #if 1
            this->addUnsupportedGrammarError(node);
            return "";
        #endif
    //
    //
    //
    //
    // ================================================

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

    directResultSymbol = nullptr;


    if (node->children.size() == 3) {

        return processExpression(node->children[1], isInGlobalScope);
    }

    auto tokenKind = node->children[0]->tokenKind;
    auto& content = node->children[0]->token.content;

    if (tokenKind == TokenKind::string_literal) {
        // 暂不支持字符串。后续应该考虑支持。

        this->addUnsupportedGrammarError(node->children[0]);
        return "";
    } else if (tokenKind == TokenKind::identifier) {

        if (isInGlobalScope) {
            
            auto& err = errorList.emplace_back();
            err.astNode = node->children[0];
            err.msg = "cannot use variable to init value in global scope. (";
            err.msg += content;
            err.msg += ")";

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
                resultValueType = symbolFromTable->valueType;

                directResultSymbol = symbolFromTable;

                return "";
            }

            // 从函数参数表找。
            auto symFromFuncParams = currentFunction->findParamSymbol(content);

            if (symFromFuncParams && symFromFuncParams->valueType != ValueType::s32) {
                
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
                resultValueType = symFromFuncParams->valueType;

                directResultSymbol = symFromFuncParams;
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
                resultValueType = symFromGlobalVar->valueType;

                directResultSymbol = symFromGlobalVar;
                return "";

            }

            errorList.emplace_back();
            auto& err = errorList.back();
            err.astNode = node->children[0];
            err.msg = "symbol not found: ";
            err.msg += content;

        }

        return "";

    } else {
        // constant
        // 暂不支持浮点。

        long long value = stoll(content);

        resultValueType = ValueType::s32;

        if (isInGlobalScope) {
            return content;
        } else {
            instructionList.push_back(__tcMakeIrInstruction(
                "mov vreg 0 imm " + content
            ));

            return "";
        }
    }

}

int tcir::IrGenerator::processDeclarationSpecifiers(
    AstNode* node, vector< TokenKind >& tokenListContainer
) {



    int prevErrors = this->errorList.size();

    if (node->children.size() != 1) {
        this->addUnsupportedGrammarError(node);

        return 1;  
    }

    if (node->children[0]->symbolKind != SymbolKind::type_specifier) {
        this->addUnsupportedGrammarError(node);

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

        return 1;
    }

    if (returnTypeSpecifier->tokenKind == TokenKind::kw_int) {
        tokenListContainer.push_back(TokenKind::kw_int);
    } else if (returnTypeSpecifier->tokenKind == TokenKind::kw_void) {
        tokenListContainer.push_back(TokenKind::kw_void);
    } else {
        this->addUnsupportedGrammarError(returnTypeSpecifier);

        return 1;
    }

    return this->errorList.size() - prevErrors;

}


string tcir::IrGenerator::symbolToIrValueCode(SymbolBase* symbol) {

    VariableSymbol* varSymbol = (VariableSymbol*) symbol;

    if (varSymbol->visibility == SymbolVisibility::global) {
        return "val " + varSymbol->name;
    } else if (varSymbol->symbolType == SymbolType::variableDefine) {
        return "val " + to_string(varSymbol->id);
    } else {
        // func param
        return "fval " + varSymbol->name;
    }

}
