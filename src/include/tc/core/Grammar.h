/*

    文法。
    part of the ToyCompile project.

    创建：2022.11.1

*/

#pragma once

#include <vector>
#include <map>
#include <string>
#include <tc/core/TokenKinds.h>

namespace tc::grammar {

    enum class SymbolType {
        
        /** 非终结符：A -> b (C) */
        NON_TERMINAL,

        /** 终结符：A -> (b) C */
        TERMINAL,

    };

    struct Symbol {
        std::string name;
        int id = -1;

        /** 标记符号是终结符或非终结符。 */
        SymbolType type;

        /** 符号类型。仅对终结符有效。 */
        TokenKind tokenKind;

        bool operator == (const Symbol& symbol) const {
            if (id >= 0 && id == symbol.id) {
                return true;
            } else {
                return this->name == symbol.name
                    && this->type == symbol.type
                    && this->tokenKind == symbol.tokenKind;
            }
        }

        bool operator != (const Symbol& symbol) const {
            return !(symbol == *this);
        }
    };

    /**
     * 表达式。
     * 
     * targetSymbol ->
     *   expressions[0][0] expression[0][1] ...
     *   | expressions[1][0] expression[1][1] ...
     *   | ...
     * 
     */
    struct Expression {
        /** 表达式唯一识别编号。在 YaccTcey 内，id 即为表达式在列表内的下标。 */
        int id;

        /** 左值。对于 A -> BC，target 是 A。 */
        int targetSymbolId;

        /**
         * 产生式。
         * 对于 A -> BC | DE，BC 和 DE 分别是一条表达式。
         * 对于产生式 BC，其表达式内共有 2 个元素，分别是 B 和 C。
         */
        std::vector< std::vector<int> > rules;

        /**
         * 
         * todo: 本实现不严谨。
         */
        bool operator == (const Expression& expression) const {
            return this->id == expression.id;
        }

        bool operator != (const Expression& expression) const {
            return !(expression == *this);
        }

    };

    /**
     *   A   -->   BcDe
     *   ^         ^^^^
     * symbol      rule
     */
    struct FlatExpression {
        int id = -1;
        int targetSymbolId;

        std::vector< int > rule;

        bool operator == (const FlatExpression& flatExpression) const;
        bool operator != (const FlatExpression& flatExpression) const;
        bool isSameWith(const FlatExpression& flatExpression) const;
    };

    /**
     * 文法。
     * 例：
     *   G(S)
     *   S -> Ab
     *   A -> cD
     *   D -> e | FG
     *   F -> ...
     * 
     * 其中，entry = S
     * 每行为 expressions 内一个元素。
     */
    struct Grammar {

        std::vector< Symbol > symbols;
        int entryId;
        std::vector< Expression > expressions;

    };

}
