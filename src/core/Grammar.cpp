// SPDX-License-Identifier: MulanPSL-2.0

/*

    文法。
    part of the ToyCompile project.

    创建：2022.11.1

*/

#include <core/Grammar.h>

using namespace std;
using namespace tc;
using namespace tc::grammar;

bool FlatExpression::operator == (const FlatExpression& flatExpression) const {
    return this->isSameWith(flatExpression);
}

bool FlatExpression::operator != (const FlatExpression& flatExpression) const {
    return !this->isSameWith(flatExpression);
}

bool FlatExpression::isSameWith(const FlatExpression& flatExpression) const {
    if (this->id >= 0 && this->id == flatExpression.id) {
        // id 相同，表达式相同。
        return true;
    } 
    
    if (this->rule.size() != flatExpression.rule.size()) {
        // 产生式长度不同，结果肯定不同。
        return false;
    } 
    
    if (this->targetSymbolId != flatExpression.targetSymbolId) {
        // 表达式归约目标不同，表达式肯定不同。
        return false;
    }

    // 逐一比较产生式。
    for (int idx = 0; idx < rule.size(); idx++) {
        if (rule[idx] != flatExpression.rule[idx]) {
            return false;
        }
    }

    // 以上检查皆通过。两个表达式确实相同。
    return true;
}
