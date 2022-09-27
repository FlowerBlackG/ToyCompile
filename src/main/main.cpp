/*
 * 程序进入点。
 * 创建于 2022年9月26日
 * 
 * 职责：解析命令行参数，启动子程序。
 */

/*

    命令行设计：
        ToyCompile (optional: s[sub program]) (0 or more: -[param name]:[param value])

    例：
        ToyCompile sLexerCli -file:in.cpp
            使用 LexerCli 子程序，将参数 file 传递给该子程序。

*/

#include <iostream>
#include <fstream>

#include <map>
#include <set>
#include <string>
#include <vector>

#include <magic_enum/magic_enum.hpp>

#include <tc/main/LexerServer/LexerServer.h>
#include <tc/core/Dfa.h>
#include <tc/core/Token.h>

#include <tc/core/Lexer.h>
#include <tc/core/TokenKinds.h>

using namespace std;

/**
 * 解析命令行参数。
 * 
 * @param argc main 函数收到的 argc。
 * @param argv main 函数收到的 argv。
 * @param paramMap 存储 params 键值对映射的 map。
 * @param paramSet 存储 params 开关的集合。
 * @param subProgramName 存储子程序名字的值。
 * @param additionalValues 存储独立参数的列表。
 * 
 * @return 是否遇到错误。如果返回为 false，表示命令行解析遇到致命问题。
 */
bool parseCli(
    int argc, const char** argv,
    map<string, string>& paramMap,
    set<string>& paramSet,
    string& subProgramName,
    vector<string>& additionalValues
) {

    if (argc < 2) {
        cout << "[Error] main: too much cli arguments." << endl;
        paramSet.insert("help");
        return false;
    }

    for (int idx = 2; idx < argc; idx++) {
        const char* & argCStr = argv[idx];
        
        if (argCStr[0] != '-') {
            additionalValues.push_back(argCStr);
            continue;
        }
        
        string arg = argCStr;
        auto colonPos = arg.find(':');
        
        if (colonPos == string::npos) {
            paramSet.insert(arg.substr(1));
            continue;
        }

        string paramKey = arg.substr(1, colonPos - 1);
        string paramVal = arg.substr(colonPos + 1);

        if (paramMap.count(paramKey)) {
            cout << "[Warning] main: redefine param key: " << paramKey << endl;
        }

        paramMap[paramKey] = paramVal;
    
    }

}

void printUsage(const char* procName = nullptr) {
    const char* outProcName = procName ? procName : "ToyCompile.exe";

}

int main(int argc, const char* argv[]) {
    map<string, string> paramMap;
    set<string> paramSet;
    string subProgramName;
    vector<string> additionalValues;

    if (!parseCli(argc, argv, paramMap, paramSet, subProgramName, additionalValues)) {
        cout << "[Error] main: failed to parse commandline arguments." << endl;
        return -1;
    }
// todo.
    TokenKind u = TokenKind::kw_typename;
    cout << magic_enum::enum_name(u) << endl;

    ifstream fin("infile.txt", ios::binary);
    if (!fin.is_open()) {
        cout << "failed to open file" << endl;
        return -2;
    }

    Lexer lexer;
    if (!lexer.dfaIsReady()) {
        cout << "lexer failed." << endl;
        return -1;
    }


    vector<Token> tkList;
    vector<LexerAnalyzeError> tkErrList;

    lexer.analyze(fin, tkList, tkErrList);
    cout << tkList.size() << endl;
    cout << tkErrList.size() << endl;

    for (auto& tk : tkList) {
        cout << "\n ---\n";
        cout << "<" << tk.row << ", " << tk.col << "> " << tk.content << endl;
        cout << "kind: " << magic_enum::enum_name(tk.kind) << " : " << int(tk.kind) << endl;
    }

    cout << "\n----++++----\n\n";

    for (auto& err : tkErrList) {
        cout << "err: " << err.dfaNodeInfo.id << endl;
        cout << "  tk: " << err.token.content << endl;
        cout << "  <" << err.row << ", " << err.col << ">\n";
    }
    
    return 0;
}
