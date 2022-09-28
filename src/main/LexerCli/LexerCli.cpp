/*
 * Lexer 本地交互工具。
 * 创建：2022年9月28日。
 */

#include <fstream>

#include <tc/core/Dfa.h>
#include <tc/core/Token.h>

#include <tc/core/Lexer.h>
#include <tc/core/TokenKinds.h>
#include <magic_enum/magic_enum.hpp>

#include "tc/main/LexerCli/LexerCli.h"

using namespace std;

void LexerCli::printUsage(ostream& out) {
    out << "Lexer Cli - local lexical tool." << endl;
    out << endl;
    out << "params:" << endl;
    out << "  fname:[x] specify input filename as x." << endl;
    out << "  help: get usage." << endl;
    out << endl;
    out << "examples:" << endl;
    out << "  LexerCli -fname:./in.cpp" << endl;
}

int LexerCli::run(
    map<string, string>& paramMap,
    set<string>& paramSet,
    vector<string>& additionalValues,
    istream& in,
    ostream& out
) {

    // 准备。
    
    if (paramSet.count("help")) {
        this->printUsage(out);
        return 0;
    }

    if (!paramMap.count("fname")) {
        out << "[Error] LexerCli: fname required." << endl;
        return -1;
    }

    string infileName = paramMap["fname"];
    ifstream fin(infileName, ios::binary);

    if (!fin.is_open()) {
        out << "[Error] LexerCli: failed to open input file." << endl;
        return -2;
    }

    // 处理。

    Lexer lexer;
    if (!lexer.dfaIsReady()) {
        out << "[Error] LexerCli: failed to init lexer." << endl;
        return -3;
    }

    vector<Token> tkList;
    vector<LexerAnalyzeError> tkErrList;
    lexer.analyze(fin, tkList, tkErrList);

    out << "symbol count: " << tkList.size() << endl;
    out << "error count : " << tkErrList.size() << endl;

    out << endl;

    for (auto& tk : tkList) {
        out << "token" << endl;
        out << "pos    : <" << tk.row << ", " << tk.col << ">" << endl;
        out << "kind   : " << tk.getKindName() << endl;
        out << "kind id: " << (unsigned) tk.kind << endl;
        out << "content: " << endl;
        out << tk.content << endl;
        out << "--- end of token ---" << endl;
    }

    for (auto& err : tkErrList) {
        out << "error" << endl;
        out << "pos    : <" << err.row << ", " << err.col << ">" << endl;
        out << "dfa sid: " << err.dfaNodeInfo.id << endl;
        out << "content: " << endl;
        out << err.token.content << endl;
        out << "--- end of error ---" << endl;
    }

    // 清理。

    fin.close();

    return 0;
}
