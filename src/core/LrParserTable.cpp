/*

    LR Parser Action Goto Table.
    Part of the ToyCompile Project.

    created on 2022.11.2

*/

#include <core/LrParserTable.h>

using namespace std;
using namespace tc;

LrParserCommand LrParserTable::getCommand(int stateId, int symbolId) {
    if (table.count(stateId)) {
        auto& row = table[stateId];
        if (row.count(symbolId)) {
            return row[symbolId];
        }
    }

    // error.
    LrParserCommand err;
    err.type = LrParserCommandType::ERROR;
    return err;
}

void LrParserTable::dump(ostream& out) {

    // primary state id
    out << "pStId " << primaryStateId << endl;
    
    // 符号表
    for (auto& sym : symbolList) {
        out << "sym " << sym.name << " " << sym.id << " "
            << int(sym.type) << " " << int(sym.tokenKind)
            << " " << int(sym.symbolKind) << endl;
    }

    // 表达式表
    for (auto& exp : flatExpressions) {
        out << "fe " << exp.id << " " << exp.targetSymbolId << " ";
        for (auto it : exp.rule) {
            out << it << " ";
        }
        out << "end" << endl;
    }

    // action goto 表
    for (auto& rowPair : table) {
        for (auto& cellPair : rowPair.second) {
            auto& cmd = cellPair.second;
            out << "tc " << rowPair.first << " " << cellPair.first << " "
                << int(cmd.type) << " " << cmd.target << endl;
        }
    }

}

static bool __lrStreamIsHealthy(istream& in) {
    return (!in.fail()) && in.good();
}

int LrParserTable::load(istream& in, ostream& msgOut) {
    // 先清空。
    primaryStateId = -1;
    symbolList.clear();
    flatExpressions.clear();
    table.clear();

    string cmd;

    try {

        while (true) {
            cmd.clear();
            in >> cmd;
            
            if (cmd.length() == 0 && !__lrStreamIsHealthy(in)) {
                break; // 读取结束。
            }

            if (cmd == "pStId") {
                // 设置 primaryStateId。
                string id;
                in >> id;
                
                primaryStateId = stoi(id);
            } else if (cmd == "sym") {
                symbolList.emplace_back();
                auto& sym = symbolList.back();
                string n, i, t, tk, sk;
                in >> n >> i >> t >> tk >> sk;
                
                sym.id = stoi(i);
                sym.name = n;
                sym.type = static_cast<grammar::SymbolType>(stoi(t));
                sym.tokenKind = static_cast<TokenKind>(stoi(tk));
                sym.symbolKind = static_cast<SymbolKind>(stoi(sk));
            } else if (cmd == "fe") {
                
                flatExpressions.emplace_back();
                auto& exp = flatExpressions.back();
                string i, ti;
                in >> i >> ti;
                
                exp.id = stoi(i);
                exp.targetSymbolId = stoi(ti);

                auto& rule = exp.rule;
                string ruleVal;
                while (true) {
                    in >> ruleVal;
                    if (ruleVal == "end") {
                        break;
                    }

                    if (!__lrStreamIsHealthy(in)) {
                        msgOut << "failed to load parser table." << endl;
                        msgOut << "fe rule not closed." << endl;
                        return -1;
                    }

                    rule.push_back(stoi(ruleVal));
                } 

            } else if (cmd == "tc") {

                string r, c, ty, tar;
                in >> r >> c >> ty >> tar;

                LrParserCommand command;
                command.target = stoi(tar);
                command.type = static_cast<LrParserCommandType>(stoi(ty));

                table[stoi(r)][stoi(c)] = command;

            } else {

                msgOut << "[warning] strange cmd: " << cmd << endl;
            
            }
            
        } // while (true)

    } catch (...) {

        // 如果 stoi 等函数出现问题，直接报错就行了。
        
        return -1;
    
    }


    return 0;
} // int LrParserTable::load(istream& in, ostream& msgOut) 
