// SPDX-License-Identifier: MulanPSL-2.0

/*

    Abstract Syntax Tree Node
    part of the ToyCompile project.

    created on 2022.11.2 at Tongji University.

*/

#pragma once

#include "core/Token.h"
#include "core/TokenKinds.h"
#include "core/SymbolKinds.h"
#include "core/Grammar.h"

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

        TokenKind& tokenKind = symbol.tokenKind;
        SymbolKind& symbolKind = symbol.symbolKind;

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

