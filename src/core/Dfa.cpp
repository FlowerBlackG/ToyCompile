/*
 * 词法分析器。
 * 创建于 2022年9月26日
 */

#include <iostream>
#include <map>
#include <vector>

#include "tc/core/Dfa.h"

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
        if (operation == "def") {

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
            
            if (isStart) {
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

/*
    for (auto& it : this->stateNodeList) {
        cout << "id   : " << it->stateInfo.id << endl;
        cout << "final: " << it->stateInfo.isFinal << endl;
        cout << "init : " << it->stateInfo.isInit << endl;
        cout << "trans: " << it->nextStates.size() << endl;
        cout << endl;
    }
*/

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
        int ch = inStream.peek();
     //   cout << "  -> in pos: " << inStream.tellg() << endl;
    //    cout << "  -> in good: " << inStream.good() << endl;
     //   cout << "  -> in fail: " << inStream.fail() << endl;

        cout << "  -> curr: " << currentNode->stateInfo.id << endl;
        cout << "  -> dfa: " << ch << ", |" << char(ch) << "|" << endl;


        DfaStateNode* nextNode = currentNode->nextState(ch);

        if (nextNode) {
      //      cout << "  -> curr: " << currentNode->stateInfo.id << endl;
            currentNode = nextNode;
            cin.get();

        } else {
            cout << "    -> dfa rec end." << endl;
            break;
        }
    }


    return currentNode;
}

/* ------------ 私有方法。 ------------ */
