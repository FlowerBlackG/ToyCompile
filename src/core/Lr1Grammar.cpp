/*

    LR 1 Grammar
    part of the ToyCompile project.
    
    created on 2022.11.1

*/

#include <tc/core/Lr1Grammar.h>

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <utility>

using namespace std;
using namespace tc;
using namespace tc::lr1grammar;

bool Lr1Expression::operator == (const Lr1Expression& other) const {
    if (this == &other) {
        // 自己与自己比较...
        return true;
    }

    if (dotPos != other.dotPos) {
        return false;
    }

    if (paimonId != other.paimonId) {
        return false;
    }

    return expressionId == other.expressionId;
}

bool Lr1Expression::operator != (const Lr1Expression& other) const {
    return !(other == *this);
}

bool State::operator == (const State& other) const {
    if (id >= 0 && id == other.id) {
        return true;
    }

    if (expressions.size() != other.expressions.size()) {
        return false;
    }

    // 比较表达式本身。
    unique_ptr<bool[]> compared(new bool[expressions.size()]);
    for (int idx = 0; idx < expressions.size(); idx++) {
        compared[idx] = false;
    }

    for (const auto& expression : expressions) {
        int idx = -1;
        while (++idx < other.expressions.size()) {
            if (!compared[idx] && other.expressions[idx] == expression) {
                compared[idx] = true;
                break;
            }
        }

        if (idx == other.expressions.size()) {
            return false;
        }
    }

    return true;
}

bool State::operator != (const State& other) const {
    return !(other == *this);
}


/* ------------ Lr1Grammar ------------ */

/* -------- 内部函数 -------- */

/**
 * 将语法中的复合表达式提取成单独的表达式。
 * 例：
 *   A -> bC | De
 * 
 * 转换为：
 *   A -> bC
 *   A -> De
 * 
 * @param grammar 语法规则。
 * @param container 存储提取结果的容器。首先会被清空。
 */
static void __lr1ExtractFlatExpressions(
    const grammar::Grammar& grammar,
    vector<grammar::FlatExpression>& flatExpressionContainer,
    unordered_map<int, vector<grammar::FlatExpression*> >& symbolExpressionsMap
) {
    auto& expMap = symbolExpressionsMap;
    auto& container = flatExpressionContainer;
    auto& symbolList = grammar.symbols;

    expMap.clear();
    container.clear();
    for (auto& expression : grammar.expressions) {
        for (auto& rule : expression.rules) {
            container.emplace_back();
            auto& exp = container.back();
            exp.targetSymbolId = expression.targetSymbolId;
            exp.rule = rule;
            exp.id = container.size() - 1;

            expMap[exp.targetSymbolId].push_back(&exp);
        }
    }
}

/**
 * 对文法进行拓广。
 */
static int __lr1ExtendGrammar(
    const int& grammarEntrySymbolId,
    vector<grammar::FlatExpression>& flatExpressions,
    vector< grammar::Symbol >& symbolList,
    unordered_map<int, vector<grammar::FlatExpression*> >& symbolExpressionsMap
) {

    flatExpressions.emplace_back();
    auto& extExp = flatExpressions.back();
    extExp.id = flatExpressions.size() - 1;
    extExp.rule.push_back(grammarEntrySymbolId);
    
    // 创建符号。
    symbolList.emplace_back();
    auto& targetSymbol = symbolList.back();
    targetSymbol.id = symbolList.size() - 1;
    targetSymbol.type = grammar::SymbolType::NON_TERMINAL;
    targetSymbol.name = "__--lr1_ext_extry--__";

    extExp.targetSymbolId = targetSymbol.id;
    symbolExpressionsMap[extExp.targetSymbolId].push_back(&extExp);
    
    return targetSymbol.id;
}

/**
 * 构建 first 集。
 * 
 * @param expressions 表达式集合。
 * @param container 存储结果的容器。
 */
static void __lr1ConstructFirstCollection(
    const vector<grammar::FlatExpression>& expressions,
    const vector< grammar::Symbol >& symbolList,
    unordered_map<int, unordered_set<int> >& container
) {
    container.clear();

    // 先将终结符的构建好。因为它比较简单。
    for (auto& expression : expressions) {
        for (auto& symbolId : expression.rule) {
            auto& symbol = symbolList[symbolId];
            if (symbol.type == grammar::SymbolType::TERMINAL) {
                container[symbolId].insert(symbolId);
            }
        }
    }
    
    // 处理非终结符的。
    // todo: 现在这种处理方式效率比较差。换成依赖树形式效率会更好。
    bool containerUpdated = true;
    while (containerUpdated) {
        containerUpdated = false;

        for (auto& expression : expressions) {

            if (expression.rule.size() == 0) {
                continue; // 理论上不会出现这种情况。出现了就跳过吧。
            }

            // 目标。显然，它是非终结符。
            int targetId = expression.targetSymbolId;
            
            auto& firstSymbolId = expression.rule[0];
            auto& firstSymbol = symbolList[firstSymbolId];

            if (firstSymbol.type == grammar::SymbolType::TERMINAL) {
                // 终结符直接插入。
                if (!container[targetId].count(firstSymbol.id)) {
                    containerUpdated = true;
                    container[targetId].insert(firstSymbol.id);
                }
            } else {
                // 非终结符，拷贝。
                // 规定文法不允许出现空字符，因此只看第一个符号就行。

                for (auto& symbolId : container[firstSymbol.id]) {
                    if (!container[targetId].count(symbolId)) {
                        containerUpdated = true;
                        container[targetId].insert(symbolId);
                    }
                }
            }

        } // for (auto& expression : expressions)

    } // while (containerUpdated)

} // internal fun __lr1ConstructFirstCollection

static int __lr1MakeEofSymbol(
    vector< grammar::Symbol >& symbolList
) {
    symbolList.emplace_back();
    auto& eofSymbol = symbolList.back();
    eofSymbol.id = symbolList.size() - 1;
    eofSymbol.name = "<eof>";
    eofSymbol.type = grammar::SymbolType::TERMINAL;
    eofSymbol.tokenKind = TokenKind::eof;

    return eofSymbol.id;
}

/**
 * 填满一个状态。
 */
static void __lr1CompleteState(
    State& state,
    const vector<grammar::FlatExpression>& flatExpressions,
    unordered_map<int, vector<grammar::FlatExpression*> >& symbolExpressionsMap,
    const vector< grammar::Symbol >& symbolList,
    unordered_map<int, unordered_set<int> >& firstCollection
) {

    /**
     * 
     * first = expression id
     * second = paimon id
     */
    set< pair<int, int> > expressionAlreadyExist;
    
    for (int expIdx = 0; expIdx < state.expressions.size(); expIdx++) {
        auto& lr1expression = state.expressions[expIdx];
        auto& flatExpression = flatExpressions[lr1expression.expressionId];
        auto& dotPos = lr1expression.dotPos;
        auto& paimonId = lr1expression.paimonId;

        if (dotPos == 0) {
            expressionAlreadyExist.insert(
                make_pair(flatExpression.id, lr1expression.paimonId)
            );
        }

        if (dotPos == flatExpression.rule.size()) {
            continue; // 已经到结尾了。
        }

        auto& nextSymbolId = flatExpression.rule[dotPos];
        auto& nextSymbol = symbolList[nextSymbolId];
        if (nextSymbol.type == grammar::SymbolType::TERMINAL) {
            continue; // 遇到终结符，不用展开。
        }

        bool nextSymbolIsLast = dotPos + 1 == flatExpression.rule.size();

        // 接下来，准备展开。
        for (auto& symbolExpressionPtr : symbolExpressionsMap[nextSymbolId]) {
            auto& symbolExpression = *symbolExpressionPtr;

            if (nextSymbolIsLast) {
                
                Lr1Expression newExp;
                newExp.dotPos = 0;
                newExp.paimonId = paimonId;
                newExp.expressionId = symbolExpression.id;

                auto infoPair = make_pair(newExp.expressionId, paimonId);

                if (!expressionAlreadyExist.count(infoPair)) {
                    expressionAlreadyExist.insert(infoPair);
                    state.expressions.push_back(newExp);
                }

                continue;

            } // if (nextSymbolIsLast) 
                
            auto& nextNextSymbolId = flatExpression.rule[dotPos + 1];
            auto& firstSymbols = firstCollection[nextNextSymbolId];
            for (auto& nextPaimonId : firstSymbols) {
                Lr1Expression newExp;
                newExp.dotPos = 0;
                newExp.paimonId = nextPaimonId;
                newExp.expressionId = symbolExpression.id;

                auto infoPair = make_pair(newExp.expressionId, paimonId);
                if (!expressionAlreadyExist.count(infoPair)) {
                    expressionAlreadyExist.insert(infoPair);
                    state.expressions.push_back(newExp);
                }

            } // for (auto& nextPaimonId : firstSymbols) {
             
        } // for (auto& symbolExpressionPtr : symbolExpressionsMap[nextSymbolId])
    } // for (int expIdx = 0; expIdx < state.expressions.size(); expIdx++)

}

static void __lr1FindToBeViewedSymbols(
    const State& state,
    const vector< grammar::FlatExpression >& flatExpressions,
    unordered_set< int >& resultContainer
) {
    for (auto& expression : state.expressions) {
        auto& expId = expression.expressionId;
        auto& dotPos = expression.dotPos;
        auto& symbolId = flatExpressions[expId].rule[dotPos];
        resultContainer.insert(symbolId);
    }
}

static void __lr1MakeState0(
    grammar::FlatExpression& entryExpression,
    int eofSymbolId,
    const vector< grammar::FlatExpression >& flatExpressions,
    unordered_map<int, vector<grammar::FlatExpression*> >& symbolExpressionsMap,
    const vector< grammar::Symbol >& symbolList,
    unordered_map<int, unordered_set<int> >& firstCollection,
    vector< State >& stateContainer
) {

    auto& states = stateContainer;

    states.clear();

    states.emplace_back();
    auto& state0 = states.back();
    state0.id = states.size() - 1;
    state0.expressions.emplace_back();
    auto& state0FirstExpression = state0.expressions.back();
    state0FirstExpression.dotPos = 0;
    state0FirstExpression.paimonId = eofSymbolId;
    state0FirstExpression.expressionId = entryExpression.id;

    // 填满该状态。
    __lr1CompleteState(
        state0, flatExpressions, symbolExpressionsMap, symbolList, firstCollection
    );
}

static int __lr1FindStateId(
    const State& state, const vector< State >& states
) {

    for (auto& existingState : states) {
        if (state == existingState) {
            return existingState.id;
        }
    }

    return -1; // 找不到。

}

static int __lr1TranslateState(
    const State& srcState,
    State& destState,
    int symbolId,
    const vector<grammar::FlatExpression>& flatExpressions,
    unordered_map<int, vector<grammar::FlatExpression*> >& symbolExpressionsMap,
    const vector< grammar::Symbol >& symbolList,
    unordered_map<int, unordered_set<int> >& firstCollection
) {

    destState.id = -1;
    destState.expressions.clear();

    for (auto& srcExp : srcState.expressions) {
        auto& flatExp = flatExpressions[srcExp.expressionId];
        if (srcExp.dotPos == flatExp.rule.size()) {
            continue; // 到达结尾了。不再移动。
        }

        auto& nextSymbolId = flatExp.rule[srcExp.dotPos];

        if (nextSymbolId != symbolId) {
            continue; // 不是想要的转移。
        }

        destState.expressions.emplace_back(srcExp);
        auto& newExp = destState.expressions.back();
        newExp.dotPos++;
    }

    __lr1CompleteState(
        destState, flatExpressions, symbolExpressionsMap, symbolList, firstCollection
    );

}

/* -------- 公有方法 -------- */
Lr1Grammar::Lr1Grammar(const grammar::Grammar& grammar) {
    this->load(grammar);
}

void Lr1Grammar::load(const grammar::Grammar& grammar) {
    // 清空原始数据。
    this->states.clear();
    this->transitions.clear();
    this->flatExpressions.clear();

    // 拷贝。
    this->symbolList = grammar.symbols;

    // 提取表达式。
    /** 符号表达式映射表。用于快速找全某个非终结符的所有产生式。 */
    unordered_map<int, vector<grammar::FlatExpression*> > symbolExpressionsMap;
    __lr1ExtractFlatExpressions(grammar, flatExpressions, symbolExpressionsMap);

    // 构造 first 集。
    unordered_map<int, unordered_set<int> > firstCollection;
    __lr1ConstructFirstCollection(flatExpressions, symbolList, firstCollection);

    // 拓广。
    entrySymbolId = __lr1ExtendGrammar(
        grammar.entryId, flatExpressions, symbolList, symbolExpressionsMap
    );

    // 引入 eof。
    this->eofSymbolId = __lr1MakeEofSymbol(symbolList);

    // 构造初始状态。
    __lr1MakeState0(
        *symbolExpressionsMap[entrySymbolId][0], eofSymbolId, flatExpressions, 
        symbolExpressionsMap, symbolList, firstCollection, states
    );

    // 转移。
    for (int stateIdx = 0; stateIdx < states.size(); stateIdx++) {
        auto& state = states[stateIdx];
        // 每个表达式的 dot 后续的符号。
        unordered_set< int > toBeViewedSymbols;
        __lr1FindToBeViewedSymbols(state, flatExpressions, toBeViewedSymbols);

        // 对于每个转移符号，计算转移。
        for (auto& transitionSymbolId : toBeViewedSymbols) {
            State nextState;
            __lr1TranslateState(
                state, nextState, transitionSymbolId, flatExpressions, 
                symbolExpressionsMap, symbolList, firstCollection
            );

            int nextStateId = __lr1FindStateId(nextState, states);
            if (nextStateId == -1) {
                nextState.id = states.size();
                states.push_back(nextState);
                nextStateId = nextState.id;
            }

            StateTransition transition;
            transition.sourceId = state.id;
            transition.targetId = nextStateId;
            transition.symbolId = transitionSymbolId;
            transitions.push_back(transition);
        }
    }

}

/* -------- 私有方法 -------- */

// 暂无。
