/*
 * 词法分析器。
 * 创建于 2022年9月26日
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "tc/core/Lexer.h"
#include "tc/core/config.h"

using namespace std;
using namespace tc;

/* ------------ 公开方法。 ------------ */

Lexer::Lexer() {
    this->prepareDfa();
}

Lexer::Lexer(ostream& msgOut) {
    this->prepareDfa(msgOut);
}

Lexer::Lexer(istream& tcdfIn) {
    this->prepareDfa(tcdfIn);
}

Lexer::Lexer(istream& tcdfIn, ostream& msgOut) {
    this->prepareDfa(tcdfIn, msgOut);
}

Lexer::~Lexer() = default;

bool Lexer::prepareDfa(ostream& msgOut) {
    ifstream fin(TC_CORE_CFG_LEXER_DFA_TCDF_PATH, ios::binary);
    if (fin.is_open()) {
        return this->dfaReady = this->prepareDfa(fin, msgOut);
    } else {
        msgOut << "[Error] Lexer: failed to open file: " 
            << TC_CORE_CFG_LEXER_DFA_TCDF_PATH 
            << endl; 
        return this->dfaReady = false;
    }
}

bool Lexer::prepareDfa() {
    ifstream fin(TC_CORE_CFG_LEXER_DFA_TCDF_PATH, ios::binary);
    
    if (fin.is_open()) {
        return this->dfaReady = this->prepareDfa(fin);
    } else {
        return this->dfaReady = false;
    }
}

bool Lexer::prepareDfa(istream& tcdfIn, ostream& msgOut) {
    if (this->prepareDfa(tcdfIn)) {
        return this->dfaReady = true;
    } else {
        msgOut << "[Error] Lexer DFA: \n" << lexDfa.errmsg << endl;
        return this->dfaReady = false;
    }
}

bool Lexer::prepareDfa(istream& tcdfIn) {
    lexDfa.build(tcdfIn);

    return this->dfaReady = lexDfa.errlevel == DfaError::DFA_OK;
}

void Lexer::analyze(
    istream& in,
    vector<Token>& tokenList,
    vector<LexerAnalyzeError>& errorList,
    bool seeCharConstantsAsNumerics
) {
    /*

        Lexer 的分析核心。

        主要工作：
          1. 借助 dfa 完成分词。
          2. 借助 map 给每个词标记类型。

        同时：
          记录行列号。
          记录错误信息。

    */

    in.clear();

    int rowNum = 1;
    int colNum = 1;

    const int READCH_FAILED = -2;

    /**
     * 读取一个字符。同时会推动 rowNum 和 colNum 的改变。
     * 
     * @param maxPos 流位置右界限。设为 -1 表示不限制。
     * 
     * @return 读取得到的字符的 ascii 码。返回 -2 (READCH_FAILED) 表示已经抵达末尾，无法读取。
     */
    const auto readCh = [&] (const streampos maxPos = -1) {
    
        while (maxPos == -1 || in.tellg() < maxPos) {

            int ch = in.get();

            if (ch == '\n') {

                colNum = 1;
                rowNum++;

            } else if (ch == '\r') {

                continue;

            } else {

                colNum++;

            }

            return ch;

        }

        return READCH_FAILED;
    };

    /* --- 拆分字符。 --- */
    
    while (true) {

        int ch;

        // 过滤空白内容。
        while (
            (ch = in.peek()) == '\n' 
            || ch == ' ' || ch == '\r'
            || ch == '\t'
        ) {

            readCh();
        }

        if (ch == EOF) {
            break;
        }

        // 登记位置并识别符号。
        auto inPos1 = in.tellg();
        auto node = lexDfa.recognize(in);
        auto inPos2 = in.tellg();

        if (node) {

            Token token;
            token.col = colNum;
            token.row = rowNum;

            // 还原位置。
            in.seekg(inPos1);

            stringstream tokenStream;

            while (in.tellg() < inPos2) {

                int chRead = readCh(inPos2);
                
                if (chRead != READCH_FAILED) {
                    tokenStream << char(chRead);
                }

            }

            string tokenString = tokenStream.str();
            
            token.content = tokenString;
            
            if (node->stateInfo.isFinal) {
                
                // 填充类型。
                this->fillTokenKind(token);

                if (seeCharConstantsAsNumerics && token.kind == TokenKind::char_constant) {
                    token.kind = TokenKind::numeric_constant;
                    token.content = to_string((int) token.content[1]);
                }
            
            } else {

                // 错误处理。

                token.kind = TokenKind::unknown;

                LexerAnalyzeError tokenError;
                tokenError.row = token.row;
                tokenError.col = token.col;
                tokenError.token = token;
                tokenError.dfaNodeInfo = node->stateInfo;

                errorList.push_back(tokenError);
            
            }

            tokenList.push_back(token);

        }
    }
}

/* ------------ 私有方法。 ------------ */

void Lexer::fillTokenKind(Token& token) {

    auto& tokenKindMap = TokenKindUtils::getInstance().tokenKindMap;

    // 判断是否是关键词。
    if (tokenKindMap.count(token.content)) {
        token.kind = tokenKindMap[token.content];
        return;
    }

    // 判断是否是数字。
    // 只要有异常抛出，就一定是无法转换成数字。
    try {
        stoll(token.content, nullptr, 0);
        token.kind = TokenKind::numeric_constant;
        return;
    } catch (...) {}

    // 单行注释么？
    if (token.content.find("//") == 0) {
        token.kind = TokenKind::single_line_comment;
        return;
    }

    // 多行注释么？
    if (token.content.find("/*") == 0) {
        token.kind = TokenKind::multi_line_comment;
        return;
    }

    // 字符串么？
    if (token.content[0] == '\"') {
        token.kind = TokenKind::string_literal;
        return;
    }

    // 字符么？
    if (token.content[0] == '\'') {
        token.kind = TokenKind::char_constant;
        return;
    }

    // 其他的都归为符号吧。
    token.kind = TokenKind::identifier;

}
