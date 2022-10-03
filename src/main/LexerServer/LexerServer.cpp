/*
 * 词法分析器服务器。
 * 创建于 2022年9月26日
 * 
 * 使用 socket 接受来自网络的分析请求，调用分析器，并传回结果。
 */

#include <memory>
#include <fstream>

#include <tc/core/Dfa.h>
#include <tc/core/Token.h>

#include <tc/core/Lexer.h>
#include <magic_enum/magic_enum.hpp>
#include <crow/crow_all.h>

#include "tc/main/LexerServer/LexerServer.h"

using namespace std;

void LexerServer::printUsage(std::ostream &out) {
    out << "Lexer Server - remote lexical tool." << endl;
    out << endl;
    out << "params:" << endl;
    out << "  port:[x] run the server on port x." << endl;
    out << "  help: get usage." << endl;
    out << endl;
    out << "examples:" << endl;
    out << "  LexerServer -port:40800" << endl;
}

void printResult(ostream &out, vector<Token> &tkList, vector<LexerAnalyzeError> &tkErrList) {
    out << "symbol count: " << tkList.size() << endl;
    out << "error count : " << tkErrList.size() << endl;

    out << endl;

    for (auto &tk: tkList) {
        out << "token" << endl;
        out << "pos    : <" << tk.row << ", " << tk.col << ">" << endl;
        out << "kind   : " << tk.getKindName() << endl;
        out << "kind id: " << (unsigned) tk.kind << endl;
        out << "content: " << endl;
        out << tk.content << endl;
        out << "--- end of token ---" << endl;
    }

    for (auto &err: tkErrList) {
        out << "error" << endl;
        out << "pos    : <" << err.row << ", " << err.col << ">" << endl;
        out << "dfa sid: " << err.dfaNodeInfo.id << endl;
        out << "content: " << endl;
        out << err.token.content << endl;
        out << "--- end of error ---" << endl;
    }
}

int LexerServer::run(std::map<std::string, std::string> &paramMap, std::set<std::string> &paramSet,
                     std::vector<std::string> &additionalValues, std::istream &in, std::ostream &out) {

    // 准备。

    if (paramSet.count("help")) {
        this->printUsage(out);
        return 0;
    }

    int port = 40800;

    if (!paramMap.count("port")) {
        out << "[INFO] LexerServer: port not found, use 40800." << endl;
    } else {
        try {
            port = stoi(paramMap["port"]);
        } catch (const exception& e) {
            out << "[ERROR] LexerServer: " << e.what() << endl;
        }
    }


    // 处理。

    Lexer lexer;
    if (!lexer.dfaIsReady()) {
        out << "[Error] LexerServer: failed to init lexer." << endl;
        return -3;
    }

    vector<Token> tkList;
    vector<LexerAnalyzeError> tkErrList;

    crow::SimpleApp app;

    CROW_ROUTE(app, "/compile")
            .websocket()
            .onopen([&](crow::websocket::connection &conn) {
                out << "new websocket connection" << endl;
            })
            .onclose([&](crow::websocket::connection &conn, const std::string &reason) {
                out << "websocket connection closed: " << reason << endl;
            })
            .onmessage([&](crow::websocket::connection &conn, const std::string &data, bool is_binary) {
                tkList.clear();
                tkErrList.clear();
                stringstream fin(data);
                lexer.analyze(fin, tkList, tkErrList);
                stringstream fout;
                printResult(fout, tkList, tkErrList);
                if (is_binary)
                    conn.send_binary(fout.str());
                else
                    conn.send_text(fout.str());
            });

    app.port(port)
            .multithreaded()
            .run();
    return 0;
}

