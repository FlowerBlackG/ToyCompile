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
        ToyCompile sLexerCli -fname:in.cpp
            使用 LexerCli 子程序，将参数 file 传递给该子程序。

*/

#include <iostream>
#include <fstream>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <tc/main/UniServer/UniServer.h>
#include <tc/main/TcSubProgram.h>
#include <tc/main/LexerCli/LexerCli.h>
#include <tc/main/ParserCli/ParserCli.h>

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
bool parseParams(
    int argc, const char** argv,
    map<string, string>& paramMap,
    set<string>& paramSet,
    string& subProgramName,
    vector<string>& additionalValues
) {

    if (argc < 2) {
        cout << "[Error] main: too less cli arguments." << endl;
        paramSet.insert("help");
        return false;
    }

    int idx = 1;
    if (argv[idx][0] == 's') {
        string arg = argv[idx];
        subProgramName = arg.substr(1);
        idx++;
    }

    for ( ; idx < argc; idx++) {
        const char* & argCStr = argv[idx];
        
        if (argCStr[0] != '-') {
            additionalValues.emplace_back(argCStr);
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

    return true;
}

/**
 * 创建一个子程序。调用者需要负责对该程序执行释放操作（delete）。
 * 
 * @param programName 程序名称。错误或空则返回默认程序。
 * @return 返回子程序指针。
 */
unique_ptr<TcSubProgram> createSubProgram(const std::string& programName) {
    
    if (programName == "UniServer") {

        return make_unique<UniServer>();

    } else if (programName == "LexerCli") {

        return make_unique<LexerCli>();

    } else if (programName == "ParserCli") {

        return make_unique<ParserCli>();

    } else {

        cout << "[Info] not subprogram specified. use LexerCli as default." << endl;
        return make_unique<LexerCli>();
    }
}

int main(int argc, const char* argv[]) {
    map<string, string> paramMap;
    set<string> paramSet;
    string subProgramName;
    vector<string> additionalValues;

    if (!parseParams(argc, argv, paramMap, paramSet, subProgramName, additionalValues)) {
        cout << "[Error] main: failed to parse commandline arguments." << endl;
        return -1;
    }

    auto subProgram = std::move(createSubProgram(subProgramName));

    int resCode = subProgram->run(
        paramMap, paramSet, additionalValues, cin, cout
    );

    return resCode;
}
