/*
 * 词法分析器服务器。
 * 创建于 2022年9月26日
 * 
 * 使用 socket 接受来自网络的分析请求，调用分析器，并传回结果。
 */

#include <memory>
#include <fstream>

#include "tc/main/UniServer/UniServer.h"
#include <tc/core/LrParserTable.h>
#include <tc/core/YaccTcey.h>
#include <tc/core/Lr1Grammar.h>
#include <tc/core/Parser.h>

#include <tc/core/Dfa.h>
#include <tc/core/Token.h>

#include <tc/core/Lexer.h>
#include <crow/crow_all.h>

using namespace std;
using namespace tc;

void UniServer::printUsage(std::ostream &out) {
    out << "Uni Server - remote lexical tool." << endl;
    out << endl;
    out << "params:" << endl;
    out << "  port:[x] run the server on port x." << endl;
    out << "  help: get usage." << endl;
    out << endl;
    out << "examples:" << endl;
    out << "  sUniServer -port:40800" << endl;
}

static void printResult(ostream &out, vector<Token> &tkList, vector<LexerAnalyzeError> &tkErrList) {
    out << "symbol count: " << tkList.size() << "\\n";
    out << "error count : " << tkErrList.size() << "\\n";

    out << "\\n";

    for (auto &tk: tkList) {
        out << "token" << "\\n";
        out << "pos    : <" << tk.row << ", " << tk.col << ">" << "\\n";
        out << "kind   : " << tk.getKindName() << "\\n";
        out << "kind id: " << (unsigned) tk.kind << "\\n";
        out << "content: " << "\\n";
        out << tk.content << "\\n";
        out << "--- end of token ---" << "\\n";
    }

    for (auto &err: tkErrList) {
        out << "error" << "\\n";
        out << "pos    : <" << err.row << ", " << err.col << ">" << "\\n";
        out << "dfa sid: " << err.dfaNodeInfo.id << "\\n";
        out << "content: " << "\\n";
        out << err.token.content << "\\n";
        out << "--- end of error ---" << "\\n";
    }
}

static void dumpNodeName(const AstNode *node, ostream &out) {

    out << "\"";

    out << node << "\\n";
    out << node->symbol.name;

    if (node->symbolType == grammar::SymbolType::TERMINAL) {
        out << "\\n" << node->token.content;
        out << "\\n(" << node->token.row << ", " << node->token.col << ")";
    }

    out << "\"";
}

static void dumpNode(const AstNode *curr, ostream &out) {
    out << "{\"name\": ";
    dumpNodeName(curr, out);
    if (!curr->children.empty()) {
        out << ", \"children\": [";
        for (auto it: curr->children) {
            dumpNode(it, out);
            if (it != curr->children.back()) {
                out << ",";
            }
        }
        out << "]";
    }
    out << "}";
}

static void dumpNodeGraph(const AstNode *curr, ostream &out) {
    for (auto it: curr->children) {
        dumpNodeGraph(it, out);
    }

    if (curr->mother != nullptr) {
        out << "{\"source\": ";
        dumpNodeName(curr->mother, out);
        out << ", \"target\":";
        dumpNodeName(curr, out);
        out << "},";
    }
}

string lexerAnalysis(vector<Token> &tokens, vector<LexerAnalyzeError> &lexerErrors, const std::string &data) {
    Lexer lexer;
    stringstream output;

    if (!lexer.dfaIsReady()) {
        output << "[Error] UniServer: failed to init lexer." << endl;
        return output.str();
    }

    tokens.clear();
    lexerErrors.clear();

    stringstream input(data);
    lexer.analyze(input, tokens, lexerErrors);
    printResult(output, tokens, lexerErrors);
    return output.str();
}

string parserAnalysis(vector<Token> &tokens, vector<LexerAnalyzeError> &lexerErrors) {
    LrParserTable table;
    stringstream output;

    // 从语法定义文件加载 action goto 表。
    YaccTcey yacc(TC_CORE_CFG_PARSER_C_TCEY_PATH);
    if (yacc.errcode != YaccTceyError::TCEY_OK) {
        output << "[error] ";
        output << yacc.errmsg << endl;
        return output.str();
    }

    auto &grammar = yacc.grammar;
    lr1grammar::Lr1Grammar lr1(grammar);
    lr1.buildParserTable(table); // 构建 action goto 表。

    /* -------- 语法识别。 -------- */

    Parser parser(table);

    vector<ParserParseError> parserErrors;
    parser.parse(tokens, parserErrors);

    if (!parserErrors.empty()) {
        for (auto &err: parserErrors) {
            output << "parser error: " << err.msg << ". ";
            if (err.tokenRelated) {
                output << "at: (" << err.token.row
                       << ", " << err.token.col << "), " << err.token.content
                       << ". ";
            }
            output << endl;
        }
        return output.str();
    }

    AstNode *astRoot = parser.getAstRoot(); // 语法树根节点。
    dumpNode(astRoot, output);
    return output.str();
}

int UniServer::run(std::map<std::string, std::string> &paramMap, std::set<std::string> &paramSet,
                   std::vector<std::string> &additionalValues, std::istream &in, std::ostream &out) {

    // 准备。

    if (paramSet.count("help")) {
        this->printUsage(out);
        return 0;
    }

    int port = 40800;

    if (!paramMap.count("port")) {
        out << "[INFO] UniServer: port not found, use 40800." << endl;
    } else {
        try {
            port = stoi(paramMap["port"]);
        } catch (const exception &e) {
            out << "[ERROR] UniServer: " << e.what() << endl;
        }
    }

    // 处理。

    vector<Token> tokens;
    vector<LexerAnalyzeError> lexerErrors;

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
                stringstream output;
                output << R"({"lexer": ")";
                output << lexerAnalysis(tokens, lexerErrors, data);
                output << R"(", "parser": )";
                output << parserAnalysis(tokens, lexerErrors);
                output << R"(})";
                if (is_binary)
                    conn.send_binary(output.str());
                else
                    conn.send_text(output.str());
            });

    app.port(port)
            .multithreaded()
            .run();
    return 0;
}

