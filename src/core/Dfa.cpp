/*
 * DFA。词法分析器依赖它。
 * 创建于 2022年9月26日
 */

#include "core/Dfa.h"

using namespace std;

/* ------------ 公开方法。 ------------ */

Dfa::Dfa() {

}

Dfa::Dfa(istream& inStream) {
    this->build(inStream);
}

Dfa::~Dfa() {
    this->clear();
}

void Dfa::build(istream& inStream) {
    this->clear();

    // 操作类型。
    // def | trans | eof
    string operation;

    while (
        inStream.good() 
        && !inStream.fail()
        && this->errlevel != DfaError::CRITICAL
    ) {

        inStream >> operation;

        if (operation == "def") { // 节点定义

            // 读入。

            int id;
            string tag;
            inStream >> id >> tag;
            if (inStream.fail()) {
                break;
            }

            // 处理标签。

            bool isNormal = tag == "normal";
            bool isFinal = tag == "final";
            bool isStart = tag == "start";

            // 检查标签是否正常。正常的话，上面三个值有且仅有1个为true。
            if (!!isNormal + !!isFinal + !!isStart != 1) {
                this->errmsg += "(d2) bad tag.\n";
                this->errlevel = DfaError::CRITICAL;
            }

            // 检查是否存在重复定义。
            if (this->stateNodeMap.count(id)) {
                this->errmsg += "(d1) duplicated node.\n";
                this->errlevel = DfaError::CRITICAL;
                break;
            }

            // 创建节点。
            DfaStateNode* node = new DfaStateNode;
            DfaStateInfo& nodeInfo = node->stateInfo;
            nodeInfo.id = id;
            nodeInfo.isFinal = isFinal;
            nodeInfo.isInit = isStart;

            // 登记节点。
            this->stateNodeList.push_back(node);
            this->stateNodeMap[id] = node;
            
            if (isStart) { // 设置进入节点。
                this->dfaEntry = node;
            }

        } else if (operation == "trans") {

            // 读入。

            int id1, id2, ascii;
            inStream >> id1 >> id2 >> ascii;

            if (inStream.fail()) {
                break;
            }

            // 寻找节点。

            DfaStateNode* id1node = nullptr;
            DfaStateNode* id2node = nullptr;
            if (this->stateNodeMap.count(id1)) {
                id1node = this->stateNodeMap[id1];
            }
            if (this->stateNodeMap.count(id2)) {
                id2node = this->stateNodeMap[id2];
            }

            if (id1node == nullptr || id2node == nullptr) {
                this->errlevel = DfaError::CRITICAL;
                this->errmsg += "(d0) node not found.\n";
                break;
            }

            // 登记转移。
            id1node->nextStates[ascii] = id2node;

        } else if (operation == "eof") { // 结束。
        
            break;

        } else {
            
            this->errlevel = DfaError::CRITICAL;

            this->errmsg += "(d3) 无法解析：";
            this->errmsg += operation;
            this->errmsg += '\n';
        
        }
    }

    if (inStream.fail()) {
        this->errlevel = DfaError::CRITICAL;
        this->errmsg += "(d4) stream failed.\n";
    }

}

void Dfa::clear() {
    this->errlevel = DfaError::DFA_OK;
    this->errmsg = "";
    this->stateNodeMap.clear();
    
    for (auto ptr : stateNodeList) {
        delete ptr;
    }

    this->stateNodeList.clear();
    this->dfaEntry = nullptr;
}


const DfaStateNode* Dfa::recognize(istream& inStream) {

    DfaStateNode* currentNode = this->dfaEntry;

    while (currentNode && inStream.good() && !inStream.fail()) {

        // 由于流的 unget 和 putback 都很不好用，此处非必要不 get。
        int ch = inStream.peek();

        // 2字节中文字符
        while (ch >= 128) {
            inStream.get();
            inStream.get();
            ch = inStream.peek();
        }
        
        // 别管 \r.
        if (ch == '\r') {
            
            inStream.get();
            continue;
        }

        DfaStateNode* nextNode = currentNode->nextState(ch);

        if (nextNode) {
            
            currentNode = nextNode; // 切换到下一状态。

            // 只有确定吃下当前字符，才真的 get。
            inStream.get();

        } else {

            // 如果读到结尾，但继续 peek，会导致流位置被设为 -1.
            if (inStream.tellg() < 0) {
                inStream.clear();
                inStream.seekg(0, ios::end);
            }
            
            // 下一状态是空的。识别结束。

            break;

        }
    }


    return currentNode;
}

/* ------------ 私有方法。 ------------ */
