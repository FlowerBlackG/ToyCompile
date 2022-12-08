/*

    统一命令行客户端。

    created on 2022.12.7

*/

#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include <tc/main/UniCli/UniCli.h>
#include <tc/utils/ConsoleColorPad.h>

#include <tc/core/Lexer.h>
#include <tc/core/Parser.h>
#include <tc/core/YaccTcey.h>
#include <tc/core/Lr1Grammar.h>
#include <tc/core/tcir/IrGenerator.h>

using namespace std;
using namespace tc;

static void __tcDumpAstNodeName(const AstNode* node, ostream& out) {

    out << "\"";

    out << node << "\\n";
    out << node->symbol.name;

    if (node->symbolType == grammar::SymbolType::TERMINAL) {
        out << "\\n" << node->token.content;
        out << "\\n(" << node->token.row << ", " << node->token.col << ")";
    }
    
    out << "\"";
}

static void __tcDumpAstNode(const AstNode* curr, ostream& out) {
    for (auto it : curr->children) {
        __tcDumpAstNode(it, out);
    }

    if (curr->mother != nullptr) {
        __tcDumpAstNodeName(curr->mother, out);
        out << " -> ";
        __tcDumpAstNodeName(curr, out);
        out << ";\n";
    }
}

static void __tcDumpAstToDot(const AstNode* root, ostream& out) {
    out << "digraph G1 {" << endl;

    __tcDumpAstNode(root, out);

    out << "}" << endl;
}

void UniCli::printUsage(std::ostream& out) {

    setOutputColor(0x2f, 0x90, 0xb9);
    out << endl;

    out << "ToyCompile Unified CommandLine" << endl;
    
    setOutputColor(0xfb, 0x99, 0x68);

    out << endl;
    out << "params:" << endl;
    setOutputColor();
    out << "  fname:[x]      : specify input file 'x'." << endl;
    out << "  help           : get help." << endl;
    out << endl;
    out << "  dump-tokens    : dump tokens." << endl;
    out << endl;
    out << "  rebuild-table  : reload parser table from tcey file." << endl;
    out << "                   if not set, parser would try to load cache" << endl;
    out << "                   to improve performance." << endl;
    out << "  no-store-table : don't store built table to file." << endl;
    out << "  cache-table:[x]: specify cache table file." << endl;
    out << "  tcey:[x]       : set tcey file 'x'." << endl;
    out << "  dump-ast       : dump parser result." << endl;
    out << "  dot-file:[x]   : store parser result to file 'x'." << endl;
    out << endl;
    out << "  dump-ir        : dump toycompile ir code." << endl;
    out << "  ir-to-file:[x] : store ir code to file." << endl;
    out << "  disable-color  : disable color to log output stream." << endl;

    setOutputColor(0xfb, 0x99, 0x68);
    out << endl;
    out << "must have:" << endl;
    setOutputColor();
    out << "  fname:[x]" << endl;

    setOutputColor(0xfb, 0x99, 0x68);
    out << endl;
    out << "examples:" << endl;
    setOutputColor();

    out << "  ParserCli -fname:resources/test/easy.c.txt"
        << " -rebuild-table" << endl;

    out << endl;
}


void UniCli::dumpTokens(
    vector<Token>& tkList, 
    ostream& out,
    bool disableColor 
) {

    bool globalColorEnable = enableOutputColor;
    if (disableColor) {
        enableOutputColor = false;
    }

    for (auto& tk : tkList) {
        setOutputColor(0x81, 0x3c, 0x85);
        out << "token" << endl;
        setOutputColor();
        out << "pos    : <" << tk.row << ", " << tk.col << ">" << endl;
        out << "kind   : " << tk.getKindName() << endl;
        out << "kind id: " << (unsigned) tk.kind << endl;
        out << "content: " << endl;
        setOutputColor(0x20, 0x89, 0x4d);
        out << tk.content << endl;
        setOutputColor();
        out << "--- end of token ---" << endl;
    }

    enableOutputColor = globalColorEnable;
}

int UniCli::lexicalAnalysis(
    map<string, string>& paramMap,
    set<string>& paramSet,
    vector<string>& additionalValues,
    ostream& logOutput,
    vector<Token>& tokenContainer
) {

    auto& out = logOutput;

    if (!paramMap.count("fname")) {

        setOutputColor(0xee, 0x3f, 0x4d);
        out << "[Error]";
        setOutputColor();
        out << " fname required." << endl;
        this->printUsage(out);
        return -1;
    }

    ifstream srcIn(paramMap["fname"], ios::binary); // 打开源文件。
    if (!srcIn.is_open()) {
        setOutputColor(0xee, 0x3f, 0x4d);
        out << "[Error] ";
        setOutputColor();
        out << "UniCli: failed to open source file." << endl;
        return -2;
    }

    Lexer lexer;

    vector<LexerAnalyzeError> lexerErrors;

    if (!lexer.dfaIsReady()) {
        
        setOutputColor(0xee, 0x3f, 0x4d);
        out << "[Error] ";
        out << "UniCli: failed to init lexer dfa." << endl;
        setOutputColor();
        return -4;
    }

    lexer.analyze(srcIn, tokenContainer, lexerErrors); // 词法分析。
    if (!lexerErrors.empty()) {
        for (auto& err : lexerErrors) {
            setOutputColor(0xee, 0x3f, 0x4d);
            out << "lexer error: ";
            setOutputColor();
            out << "(" << err.row
                << ", " << err.col << ") "
                << err.msg << ". token: "
                << err.token.content << "." << endl;
        }

        return -5;
    }

    srcIn.close();

    if (paramSet.count("dump-tokens")) {
        this->dumpTokens(tokenContainer, out);
    }

    return 0;
}

int UniCli::syntaxAnalysis(
    map<string, string>& paramMap,
    set<string>& paramSet,
    vector<string>& additionalValues,
    ostream& logOutput,
    vector<Token>& tokens,
    Parser& parser
) {

    bool rebuildTable = paramSet.count("rebuild-table");
    bool noStoreTable = paramSet.count("no-store-table");

    auto& out = logOutput;

    /* -------- 准备语法识别器。 -------- */

    // 尝试加载语法分析缓存。 
    
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

    // 准备 action goto 表。

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

    parser.loadParserTable(table);

    vector<ParserParseError> parserErrors;
    parser.parse(tokens, parserErrors);

    if (!parserErrors.empty()) {
        for (auto& err : parserErrors) {
            
            setOutputColor(0xee, 0x27, 0x46);

            out << "parser error: ";

            setOutputColor();
            

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

    AstNode* astRoot = parser.getAstRoot();

    // 输出结果。按照 graphviz dot 格式输出。
    if (paramSet.count("dump-ast")) {
        bool storeResToFile = paramMap.count("dot-file");
        bool dumpedToFile = false;

        if (storeResToFile) {
            ofstream fout(paramMap["dot-file"], ios::binary);
            if (fout.is_open()) {
                dumpedToFile = true;
                __tcDumpAstToDot(astRoot, fout);
            }
        }

        if (!dumpedToFile) {
            __tcDumpAstToDot(astRoot, out);
        }
    }

    return 0;
}

int UniCli::generateTcIr(
    map<string, string>& paramMap,
    set<string>& paramSet,
    vector<string>& additionalValues,
    ostream& logOutput,
    AstNode* astRoot,
    tcir::IrGenerator& irGenerator
) {

    auto& irGen = irGenerator;
    auto& out = logOutput;

    // 输入语法树，内部产生中间代码（及错误分析）。
    irGen.process(astRoot); 

    auto& irErrors = irGen.getErrorList();
    auto& irWarnings = irGen.getWarningList();

    /**
     * 输出某个语法树节点的详细信息。包含其内部第一个终结符的内容和位置。
     */
    auto printErrTokenDetail = [this, &out] (tcir::IrGeneratorError& err) {

        AstNode* tk = err.astNode;
        while (tk->symbolType != grammar::SymbolType::TERMINAL) {
            tk = tk->children[0];
        }

        this->setOutputColor(0xbc, 0x84, 0xa8);
        out << "  token: ";
        this->setOutputColor();
        out << tk->token.content << endl;

        this->setOutputColor(0x80, 0x6d, 0x9e);
        out << "  loc  : ";
        this->setOutputColor();
        out << "(" << tk->token.row << ", " << tk->token.col << ")" << endl;
        this->setOutputColor();
    };

    // 打印 warnings
    for (auto& it : irWarnings) {
        ConsoleColorPad::setColor(0xfc, 0xa1, 0x06);
        cout << "warning: ";
        ConsoleColorPad::setColor();
        cout << it.msg << endl;
        printErrTokenDetail(it);
    }

    // 打印 errors
    for (auto& it : irErrors) {
        ConsoleColorPad::setColor(0xde, 0x1c, 0x31);
        cout << "error: ";
        ConsoleColorPad::setColor();
        cout << it.msg << endl;
        printErrTokenDetail(it);
    }

    if (!irErrors.empty()) {
        return -7;
    }

    /* 输出中间代码。 */
    if (paramSet.count("dump-ir")) {

        bool toFile = paramMap.count("ir-to-file");
        if (toFile) {
            ofstream fout(paramMap["ir-to-file"], ios::binary);
            if (!fout.is_open()) {
                setOutputColor(0xee, 0x3f, 0x4d);
                out << "[Error] ";
                setOutputColor();
                out << "failed to open ir output file." << endl;
                return -9;
            }

            irGen.dump(fout, false);
            fout.close();

        } else {
            
            irGen.dump(out, this->enableOutputColor);
        
        }

    }

    return 0;
}

int UniCli::run(
    map<string, string>& paramMap,
    set<string>& paramSet,
    vector<string>& additionalValues,
    istream& in,
    ostream& out
) {

    // 基本的命令行参数处理。
    
    if (paramSet.count("disable-color")) { // 终端颜色开关。
        this->enableOutputColor = false;
    }

    
    if (paramSet.count("help")) { // 打印程序使用说明。
        this->printUsage(out);
        return 0;
    }

    /* -------- 词法识别。 -------- */

    
    vector<Token> tokens;
    int resCode = lexicalAnalysis(
        paramMap, paramSet, additionalValues, out, tokens
    );

    if (resCode) {
        return resCode;
    }
    

    /* -------- 语法识别。 -------- */

    Parser parser;
    resCode = syntaxAnalysis(
        paramMap, paramSet, additionalValues, out, tokens, parser
    );

    if (resCode) {
        return resCode;
    }

    /* -------- 语义分析。 -------- */

    tcir::IrGenerator irGen;
    resCode = generateTcIr(
        paramMap, paramSet, additionalValues, out, parser.getAstRoot(), irGen
    );

    if (resCode) {
        return resCode;
    }

    // todo: 中间代码转汇编。

    return 0;

}

void UniCli::setOutputColor(int red, int green, int blue) {
    if (enableOutputColor) {
        ConsoleColorPad::setColor(red, green, blue);
    }
}

void UniCli::setOutputColor() {
    if (enableOutputColor) {
        ConsoleColorPad::setColor();
    }
}
