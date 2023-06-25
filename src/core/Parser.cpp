// SPDX-License-Identifier: MulanPSL-2.0

/*

    Parser.h
    part of the ToyCompile project.
    created on 2022.11.2

*/

#include <core/Parser.h>

using namespace std;
using namespace tc;

Parser::Parser() {

}

Parser::Parser(const LrParserTable& parserTable) {
    this->loadParserTable(parserTable);
}

void Parser::loadParserTable(const LrParserTable& parserTable) {
    this->parserTable = parserTable;
}

int Parser::parse(
    vector< Token >& tokens,
    vector< ParserParseError >& errorList
) {

    this->clear();

    // 状态栈。
    vector< int > states;

    // 符号栈。
    vector< AstNode* > nodes;

    // 提取分析表内的元素。
    auto& table = parserTable.table;
    auto& symbolList = parserTable.symbolList;
    auto& flatExpressions = parserTable.flatExpressions;

    // 构建 token kind -> symbol id 映射表。
    unordered_map<TokenKind, int> tokenKindToSymbolIdMap;
    for (auto& symbol : symbolList) {
        if (symbol.type == grammar::SymbolType::NON_TERMINAL) {
            continue;
        }

        tokenKindToSymbolIdMap[symbol.tokenKind] = symbol.id;
    }

    int currentTokenIdx = 0;
    int tokenListSize = tokens.size();

    // 初状态。
    states.push_back(parserTable.primaryStateId);

    int errorCount = 0;

    while (true) {
        
        /*
            过程：
              1. 看待进入符号。此时，状态栈比符号栈尺寸多 1。
              
              2. 如果出错，就结束。
              2. 如果接受，就结束。同时寻找祖先节点。
              2. 如果是 shift，就符号进入，状态进入。
              2. 如果是 reduce，就合并。
                3. 尝试移动 1 个状态。失败就结束。成功就继续。
        */


        // 异常结尾。
        if (currentTokenIdx == tokenListSize) {
            errorList.emplace_back();
            ParserParseError& error = errorList.back();
            error.tokenRelated = false;
            error.msg = "unexpected end of tokens.";
            errorCount++;
            
            break;
        }

        // 下一个字符。当然，它是终结符。
        auto& token = tokens[currentTokenIdx];

        if (token.kind == TokenKind::multi_line_comment 
            || token.kind == TokenKind::single_line_comment
        ) {
            currentTokenIdx++;
            continue; // 忽略注释。
        }

        auto symbolId = tokenKindToSymbolIdMap[token.kind];

        // 指令。
        auto command = parserTable.getCommand(states.back(), symbolId);

        // 错误。
        if (command.type == LrParserCommandType::ERROR) {
            errorList.emplace_back();
            auto& err = errorList.back();
            err.tokenRelated = true;
            err.token = token;
            err.msg = "(";
            err.msg.append( to_string(token.row) )
                .append( ", " )
                .append( to_string(token.col) )
                .append( ") " )
                .append( "unexpected token: " )
                .append( token.content );

            errorCount++;

            break;
        }

        // 接受。
        if (command.type == LrParserCommandType::ACCEPT) {
            if (!nodes.empty()) {
                astRoot = nodes[0];
                while (astRoot->mother != nullptr) {
                    astRoot = astRoot->mother;
                }
            }

            break;
        }

        // goto。不应该出现。
        if (command.type == LrParserCommandType::GOTO) {
            errorList.emplace_back();
            auto& err = errorList.back();
            err.tokenRelated = true;
            err.token = token;
            err.msg = "(";
            err.msg.append( to_string(token.row) )
                .append( ", " )
                .append( to_string(token.col) )
                .append( ") " )
                .append("internal error: unexpected command GOTO. token: ")
                .append( token.content );

            errorCount++;

            break;
        }

        // shift.
        if (command.type == LrParserCommandType::SHIFT) {
            currentTokenIdx++;
            AstNode* node = new AstNode;
            nodes.push_back(node);

            node->symbol = symbolList[symbolId];
            node->token = token;
            states.push_back(command.target);

            continue;
        }


        // reduce.
        
        const auto& expression = flatExpressions[command.target];

        AstNode* reducedNode = new AstNode; // 归约得到的节点。
        reducedNode->symbol = symbolList[expression.targetSymbolId];

        int ruleSize = expression.rule.size();
        
        for (int nodeIdx = nodes.size() - ruleSize; nodeIdx < nodes.size(); nodeIdx++) {
            auto node = nodes[nodeIdx];
            node->mother = reducedNode;
            reducedNode->children.push_back(node);
        }

        for (int counter = 0; counter < ruleSize; counter++) {
            nodes.pop_back();
            states.pop_back();
        }

        nodes.push_back(reducedNode);

        // 移动 1 个状态。
        auto gotoCommand = parserTable.getCommand(states.back(), reducedNode->symbol.id);

        if (gotoCommand.type == LrParserCommandType::GOTO) {

            states.push_back(gotoCommand.target);

        } else if (gotoCommand.type == LrParserCommandType::ERROR) {

            errorList.emplace_back();
            auto& err = errorList.back();
            err.tokenRelated = true;
            err.token = token;
            err.msg = "(";
            err.msg.append( to_string(token.row) )
                .append( ", " )
                .append( to_string(token.col) )
                .append( ") " )
                .append( "unexpected token: " )
                .append( token.content );

            errorCount++;

            break;

        } else {
            errorList.emplace_back();
            auto& err = errorList.back();
            err.tokenRelated = true;
            err.token = token;
            err.msg = "(";
            err.msg.append( to_string(token.row) )
                .append( ", " )
                .append( to_string(token.col) )
                .append( ") " )
                .append("internal error: unexpected command. token: ")
                .append( token.content );

            errorCount++;

            break;
        }

    }

    return errorCount;
}

void Parser::clear() {
    if (this->astRoot != nullptr) {
        delete this->astRoot;
        this->astRoot = nullptr;
    }
}

Parser::~Parser() {
    this->clear();
}
