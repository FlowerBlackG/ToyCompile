/*
 * 词法分析器服务器。
 * 创建于 2022年9月26日
 * 
 * 使用 socket 接受来自网络的分析请求，调用分析器，并传回结果。
 */

#include <memory>
#include <fstream>

#include <tc/main/LexerServer/LexerServer.h>
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

static void printResult(ostream &out, vector<Token> &tkList, vector<LexerAnalyzeError> &tkErrList) {
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

static void dumpNodeName(const AstNode* node, ostream& out) {

    out << "\"";

    out << node << "\\n";
    out << node->symbol.name;

    if (node->symbolType == grammar::SymbolType::TERMINAL) {
        out << "\\n" << node->token.content;
        out << "\\n(" << node->token.row << ", " << node->token.col << ")";
    }

    out << "\"";
}

static void dumpNode(const AstNode* curr, ostream& out) {
    for (auto it : curr->children) {
        dumpNode(it, out);
    }

    if (curr->mother != nullptr) {
        out << "{\"source\": ";
        dumpNodeName(curr->mother, out);
        out << ", \"target\":";
        dumpNodeName(curr, out);
        out << "},";
    }
}

stringstream lexerAnalysis(vector<Token> &tokens, vector<LexerAnalyzeError> &lexerErrors, const std::string &data) {
    Lexer lexer;
    stringstream output;

    if (!lexer.dfaIsReady()) {
        output << "[Error] LexerServer: failed to init lexer." << endl;
        return output;
    }

    tokens.clear();
    lexerErrors.clear();

    stringstream input(data);
    lexer.analyze(input, tokens, lexerErrors);
    printResult(output, tokens, lexerErrors);
    return output;
}

stringstream parserAnalysis(vector<Token> &tokens, vector<LexerAnalyzeError> &lexerErrors) {
    LrParserTable table;
    stringstream output;

    // 从语法定义文件加载 action goto 表。
    YaccTcey yacc(TC_CORE_CFG_PARSER_C_TCEY_PATH);
    if (yacc.errcode != YaccTceyError::TCEY_OK) {
        output << "[error] ";
        output << yacc.errmsg << endl;
        return output;
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
        return output;
    }

    AstNode *astRoot = parser.getAstRoot(); // 语法树根节点。
    output << "[";
    dumpNode(astRoot, output);
    output << "]";
    return output;
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
        } catch (const exception &e) {
            out << "[ERROR] LexerServer: " << e.what() << endl;
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
                lexerAnalysis(tokens, lexerErrors, data);
                output << R"(", "parser": )";
                parserAnalysis(tokens, lexerErrors);
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

