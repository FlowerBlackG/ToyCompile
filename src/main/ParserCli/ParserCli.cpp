// SPDX-License-Identifier: MulanPSL-2.0

/*

    语法分析器。
    创建：2022.11.1

*/

#include <main/ParserCli/ParserCli.h>
#include <core/YaccTcey.h>
#include <core/Lr1Grammar.h>
#include <core/config.h>

#include <core/Lexer.h>
#include <core/Parser.h>

#include <fstream>
#include <iostream>

#include <utils/ConsoleColorPad.h>

using namespace std;
using namespace tc;

/*

  Parser 工作原理与缓存优化详见 tc/core/Parser.h

*/

void ParserCli::printUsage(std::ostream &out) {
    out << "ToyCompile Parser CommandLine." << endl;
    out << endl;
    out << "params:" << endl;
    out << "  fname:[x]      : specify input file 'x'." << endl;
    out << "  help           : get help." << endl;
    out << "  rebuild-table  : reload parser table from tcey file." << endl;
    out << "                   if not set, parser would try to load cache" << endl;
    out << "                   to improve performance." << endl;
    out << "  no-store-table : don't store built table to file." << endl;
    out << "  cache-table:[x]: specify cache table file." << endl;
    out << "  tcey:[x]       : set tcey file 'x'." << endl;
    out << "  dot-file:[x]   : store result to file 'x'." << endl;

    out << endl;
    out << "must have:" << endl;
    out << "  fname:[x]" << endl;

    out << endl;
    out << "examples:" << endl;
    out << "  ParserCli -fname:resources/test/easy.c.txt"
        << " -rebuild-table" << endl;

}

static void __tcDumpNodeName(const AstNode* node, ostream& out) {

    out << "\"";

    out << node << "\\n";
    out << node->symbol.name;

    if (node->symbolType == grammar::SymbolType::TERMINAL) {
        out << "\\n" << node->token.content;
        out << "\\n(" << node->token.row << ", " << node->token.col << ")";
    }
    
    out << "\"";
}

static void __tcDumpNode(const AstNode* curr, ostream& out) {
    for (auto it : curr->children) {
        __tcDumpNode(it, out);
    }

    if (curr->mother != nullptr) {
        __tcDumpNodeName(curr->mother, out);
        out << " -> ";
        __tcDumpNodeName(curr, out);
        out << ";\n";
    }
}

static void __tcDumpToDot(const AstNode* root, ostream& out) {
    out << "digraph G1 {" << endl;

    __tcDumpNode(root, out);

    out << "}" << endl;
}

int ParserCli::run(
    map<string, string>& paramMap,
    set<string>& paramSet,
    vector<string>& additionalValues,
    istream& in,
    ostream& out
) {
    // 命令行处理。
    if (paramSet.count("help")) {
        this->printUsage(out);
        return 0;
    }

    if (!paramMap.count("fname")) {
        out << "[Error] ParserCli: fname required." << endl;
        this->printUsage(out);
        return -1;
    }

    bool rebuildTable = paramSet.count("rebuild-table");
    bool noStoreTable = paramSet.count("no-store-table");

    
    string tceyFilePath;
    if (paramMap.count("tcey")) {
        tceyFilePath = paramMap["tcey"];
    } else {
        tceyFilePath = TC_CORE_CFG_PARSER_C_TCEY_PATH;
    }

    string cacheTableFilePath;
    if (paramMap.count("cache-table")) {
        cacheTableFilePath = paramMap["cache-table"];
    } else {
        cacheTableFilePath = tceyFilePath;
        cacheTableFilePath += ".tcpt";
    }

    /* -------- 词法识别。 -------- */

    ifstream srcIn(paramMap["fname"], ios::binary); // 打开源文件。
    if (!srcIn.is_open()) {
        out << "[Error] ParserCli: failed to open source file." << endl;
        return -2;
    }

    Lexer lexer;

    vector<LexerAnalyzeError> lexerErrors;
    vector<Token> tokens;

    if (!lexer.dfaIsReady()) {
        out << "[Error] ParserCli: failed to init lexer dfa." << endl;

        return -4;
    }

    lexer.analyze(srcIn, tokens, lexerErrors); // 词法分析。
    if (!lexerErrors.empty()) {
        for (auto& err : lexerErrors) {
            out << "lexer error: (" << err.row
                << ", " << err.col << ") "
                << err.msg << ". token: "
                << err.token.content << "." << endl;
        }

        return -5;
    }

    srcIn.close();

    /* -------- 准备语法识别器。 -------- */

    // 准备符号表。

    // 首先尝试加载缓存。

    LrParserTable table;
    bool tableLoadedFromCache = false; // 是否成功从缓存加载。

    if (!rebuildTable && !tableLoadedFromCache) {
        ifstream fin(cacheTableFilePath, ios::binary);
        int loadResult;

        if (fin.is_open()) {
        
            loadResult = table.load(fin, out);
            tableLoadedFromCache = loadResult == 0;
        
        } else {
        
            out << "[warn] failed to open cache: " << cacheTableFilePath << endl;
        
        }

        if (!tableLoadedFromCache) {
            out << "[warn] failed to load table. error code: " << loadResult << endl;
        }
    }

    // 从语法定义文件加载 action goto 表。
    if (!tableLoadedFromCache) {
        YaccTcey yacc(tceyFilePath);
        if (yacc.errcode != YaccTceyError::TCEY_OK) {
            out << "[error] ";
            out << yacc.errmsg << endl;
            return -3;
        }

        auto& grammar = yacc.grammar;

        lr1grammar::Lr1Grammar lr1(grammar);

        lr1.buildParserTable(table); // 构建 action goto 表。
    }

    if (!noStoreTable) {
        // 保存表到文件。

        ofstream fout(cacheTableFilePath, ios::binary);
        if (fout.is_open()) {
            table.dump(fout);
            
        } else {
            out << "[warn] failed to store parser table." << endl;
        }
    }

    /* -------- 语法识别。 -------- */

    Parser parser(table);

    vector<ParserParseError> parserErrors;
    parser.parse(tokens, parserErrors);

    if (!parserErrors.empty()) {
        for (auto& err : parserErrors) {
            
            if (&out == &cout) {
                ConsoleColorPad::setColor(0xee, 0x27, 0x46);
            }

            out << "parser error: ";

            if (&out == &cout) {
                ConsoleColorPad::setColor();
            }

            out << err.msg << ". ";

            if (err.tokenRelated) {
                out << "at: (" << err.token.row
                    << ", " << err.token.col << "), " << err.token.content
                    << ". ";
            }

            out << endl;

        }
        return -6;
    }

    AstNode* astRoot = parser.getAstRoot(); // 语法树根节点。

    // 输出结果。按照 graphviz dot 格式输出。
    bool storeResToFile = paramMap.count("dot-file");
    bool dumpedToFile = false;

    if (storeResToFile) {
        ofstream fout(paramMap["dot-file"], ios::binary);
        if (fout.is_open()) {
            dumpedToFile = true;
            __tcDumpToDot(astRoot, fout);
        }
    }

    if (!dumpedToFile) {
        __tcDumpToDot(astRoot, out);
    }

    return 0;

}
