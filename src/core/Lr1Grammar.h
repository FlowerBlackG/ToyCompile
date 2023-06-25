// SPDX-License-Identifier: MulanPSL-2.0

/*

    LR 1 Grammar
    part of the ToyCompile project.
    
    created on 2022.11.1

*/

#pragma once

#include <core/Grammar.h>
#include <core/LrParserTable.h>
#include <vector>

namespace tc::lr1grammar {

    /**
     * LR1 文法表达式。
     * 即：LR1 项目。
     * 
     * 例：
     *   A -> aA·Cd, #
     * 
     */
    struct Lr1Expression {
        /**
         * 原始表达式 id。
         * 对应一条 flat expression。
         * 例：A -> aACd
         */
        int expressionId; 

        /**
         * 观察点所在位置。该位置为观察点所在的间隙位置。
         * 如：
         *   A -> aA·Cd, e 中，dotPos = 2。
         */
        int dotPos = -1;

        /**
         * 展望字符的符号 id。
         * 如：
         *   A -> aA·Cd, e 中，paimon = e
         * 
         * 显然，paimon 必须是终结符。也可以是结束符号 (#)。
         * 
         * paimon 名字来自《原神》，它永远跟随在主角身后。
         */
        int paimonId;

        bool operator == (const Lr1Expression& other) const;
        bool operator != (const Lr1Expression& other) const;
    };

    /**
     * LR1状态。
     * 由多个 LR1 项目共同组成。
     */
    struct State {

        /**
         * 状态编号。需要是不小于 0 的整数。
         */
        int id = -1;

        /**
         * 状态内的表达式集合。
         */
        std::vector< Lr1Expression > expressions;
        
        bool operator == (const State& other) const;
        bool operator != (const State& other) const;
    };


    /**
     * LR1 文法。
     */
    class Lr1Grammar {
    public: // 公有方法。
        Lr1Grammar(const grammar::Grammar& grammar);

        /**
         * 加载文法。通过输入的语法，构建 LR1 文法。
         * 
         * @param grammar 输入的文法。要求每个符号已经唯一地编号。
         */
        void load(const grammar::Grammar& grammar);

        /**
         * 根据本 LR1 文法，构建 Action Goto 表。
         * 
         * @param table 构建结果。
         */
        void buildParserTable(LrParserTable& table);

    public: // getters
        const std::vector< grammar::Symbol >& getSymbolList() {
            return this->symbolList;
        }
        
        const std::vector< grammar::FlatExpression >& getFlatExpressions () {
            return this->flatExpressions;
        }

        int getEntrySymbolId() { return this->entrySymbolId; }
        int getEofSymbolId() { return this->eofSymbolId; }

    protected: // 私有成员。
        /**
         * 状态集。即，项目集的集合。
         * 项目集编号从 0 开始。即：states[0].id = 0 .
         */
        std::vector< State > states;

        /**
         * GO 函数列表。
         * 
         * int1 -> < int2 -> int3 >:
         *   int1: 当前状态 id
         *   int2: 造成转移的符号的 id
         *   int3: 转移到达的状态 id
         */
        std::unordered_map<int, std::unordered_map<int, int> > transitionMap;

        /**
         * 文法符号表。符号 id 和其在表内下标保持一致。
         */
        std::vector< grammar::Symbol > symbolList;

        /**
         * 文法产生式列表。产生式 id 和其在表内下标保持一致。
         */
        std::vector< grammar::FlatExpression > flatExpressions;

        /**
         * 文法进入符号的 id。该符号应该是拓广后产生的。
         */
        int entrySymbolId = -1;

        /**
         * 文件结尾符号（eof）的 id。
         */
        int eofSymbolId = -1;

    protected: // 私有方法。
        // 暂无。

    private:
        Lr1Grammar() {}
        Lr1Grammar(const Lr1Grammar&) {}
        const Lr1Grammar& operator = (const Lr1Grammar&) { return *this; }
    };

};
