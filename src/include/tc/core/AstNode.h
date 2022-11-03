/*

    Abstract Syntax Tree Node
    part of the ToyCompile project.

    created on 2022.11.2 at Tongji University.

*/

#pragma once

#include <tc/core/Token.h>
#include <tc/core/TokenKinds.h>
#include <tc/core/Grammar.h>

#include <vector>

namespace tc {

    /**
     * 语法树节点。
     */
    struct AstNode {

        /**
         * 上级节点。
         */
        AstNode* mother = nullptr;

        /**
         * 子节点。
         */
        std::vector< AstNode* > children;

        /**
         * 符号。
         */
        grammar::Symbol symbol;

        /**
         * 符号类型。
         */
        grammar::SymbolType& symbolType = symbol.type;

        /**
         * 终结符。仅当符号为终结符时可用。
         */
        Token token;

        
        ~AstNode();
        
        /**
         * 释放所有孩子结点的内存。递归释放。
         */
        void freeChildren();
        
    };

}

