/*

    LR 1 Grammar
    part of the ToyCompile project.
    
    created on 2022.11.1

*/

#pragma once

#include <tc/core/Grammar.h>
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
         * 
         * 对应一条 flat expression。
         */
        int expressionId; 

        /**
         * 观察点所在位置。该位置为观察点所在的间隙位置。
         * 如：
         *   A -> aA·Cd, e 中，dotPos = 2。
         */
        int dotPos;

        /**
         * 展望字符。
         * 如：
         *   A -> aA·Cd, e 中，paimon = e
         * 
         * 显然，paimon 必须是终结符。也可以是结束符号 (#)。
         * 
         * paimon 原意为派蒙。在《原神》中，它永远跟随在主角身后。
         * 
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
        int id = -1;
        std::vector< Lr1Expression > expressions;
        
        bool operator == (const State& other) const;
        bool operator != (const State& other) const;
    };

    /**
     * 状态转换描述信息。
     * 
     * 例：
     *   I1     --B->     I2
     *    ^       ^        ^
     *  source  symbol  target
     */
    struct StateTransition {
        int sourceId;
        int targetId;
        int symbolId;
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

    protected: // 私有成员。
        /**
         * 状态集。即，项目集的集合。
         * 项目集编号从 0 开始。即：states[0].id = 0 .
         */
        std::vector< State > states;

        /**
         * GO 函数列表。
         */
        std::vector< StateTransition > transitions;

        std::vector< grammar::Symbol > symbolList;
        std::vector< grammar::FlatExpression > flatExpressions;
        int entrySymbolId = -1;
        int eofSymbolId = -1;

    protected: // 私有方法。
        

    private:
        Lr1Grammar() {}
        Lr1Grammar(const Lr1Grammar&) {}
        const Lr1Grammar& operator = (const Lr1Grammar&) { return *this; }
    };

};
