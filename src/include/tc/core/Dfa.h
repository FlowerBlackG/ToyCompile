/*
 * DFA。
 * 创建于 2022年9月26日
 */

#pragma once

#include <map>
#include <string>
#include <vector>
#include <set>
#include <iostream>

/**
 * DFA 节点信息。
 * 记录一个节点的基本信息。
 */
struct DfaStateInfo {
    int id;
    std::string name;
    bool isInit;
    bool isFinal;

    bool isNormal() {
        return !(isFinal || isInit);
    }
};

/**
 * DFA 状态节点。
 * 包含节点的基本信息，以及允许转移的节点号表。
 */
struct DfaStateNode {

    /** 节点基本信息。 */
    DfaStateInfo stateInfo;

    /** 
     * 允许到达的状态。
     * 如果状态 id 不在集合里，则达不到下一状态。 
     * 
     * 映射：id: int -> node: DfaStateNode*
     */
    std::map<int, DfaStateNode*> nextStates;

    /**
     * 获取下一状态。
     * 
     * @param ascii 转移所用字符。
     * @return 若无法转移，则返回 nullptr。
     */
    DfaStateNode* nextState(int ascii) {
        return nextStates.count(ascii) ? nextStates[ascii] : nullptr;
    }
};

enum class DfaError {
    DFA_OK,

    /** 严重错误，导致 DFA 不可用。 */
    CRITICAL,

    /** 一般错误。生成的 DFA 可能与预期不符。 */
    WARNING
};

/**
 * DFA。
 * 使用 tcdf 格式命令构建。
 * .tcdf 格式说明参考：
 *   tools/ToyCompileToolsKt/src/main/kotlin/Jff2Tcdf.kt
 */
class Dfa {

public:

    /* ------------ 公开方法。 ------------ */

    Dfa();
    Dfa(std::istream& inStream);
    ~Dfa();

    /**
     * 使用 tcdf 格式命令构建 DFA。
     * 
     * 构建完毕，需要检查 errlevel。若非 DFA_OK，则需处理错误情况。
     * 
     * @param inStream 输入流。内含构建命令。
     */
    void build(std::istream& inStream);
    void clear();

    /**
     * 识别语言。
     * 若到达异常状态，则返回错误前的状态。本方法会改变流的位置等情况。
     * 
     * @param inStream 输入流。
     * @return 到达节点。到达错误节点，返回 nullptr。返回的节点可能是非终态的。 
     */
    const DfaStateNode* recognize(std::istream& inStream);

protected:

    /* ------------ 私有方法。 ------------ */

protected:

    /* ------------ 私有成员。 ------------ */

    std::vector<DfaStateNode*> stateNodeList;
    std::map<int, DfaStateNode*> stateNodeMap;

    DfaStateNode* dfaEntry = nullptr;

public:

    /* ------------ 对外开放的成员。 ------------ */

    /** 错误等级。 */
    DfaError errlevel = DfaError::DFA_OK;

    /** 报错信息。 */
    std::string errmsg;


private:
    Dfa(const Dfa& dfa) {}

};
