/*
 * 词法分析器。
 * 创建于 2022年9月26日
 */

#pragma once

#include <iostream>
#include <vector>

#include <core/Dfa.h>
#include <core/Token.h>

namespace tc {

    struct LexerAnalyzeError {

        /** 报错位置：行号。 */
        int row;

        /** 报错位置：列号。 */
        int col;

        /** 报错的符号。 */
        Token token;

        /** 最后的 dfa 节点位置。可以用于在自动机内定位。 */
        DfaStateInfo dfaNodeInfo;

        /** 报错信息。可以用于输出。 */
        std::string msg;
    };

    /**
     * 词法分析器核心。
     * 
     */
    class Lexer {

    public:
        
        /* ------------ 公开方法。 ------------ */

        Lexer();
        Lexer(std::ostream& msgOut);
        Lexer(std::istream& tcdfIn);
        Lexer(std::istream& tcdfIn, std::ostream& msgOut);
        ~Lexer();

        bool prepareDfa(std::ostream& msgOut);
        bool prepareDfa();
        bool prepareDfa(std::istream& tcdfIn, std::ostream& msgOut);
        bool prepareDfa(std::istream& tcdfIn);

        inline bool dfaIsReady() { return dfaReady; }

        /**
         * 词法分析。
         * 
         * @param in 字符输入流。应该指向文件内容的开头。
         * @param tokenList 存储分析结果的列表容器。
         */
        void analyze(
            std::istream& in,
            std::vector<Token>& tokenList,
            std::vector<LexerAnalyzeError>& errorList,
            bool seeCharConstantsAsNumerics = false
        );

    protected:

        /* ------------ 私有方法。 ------------ */

        void fillTokenKind(Token& token);

    protected:

        /* ------------ 私有成员。 ------------ */
        
        Dfa lexDfa;
        bool dfaReady = false;

    private:
        Lexer(const Lexer&) {}

    };

}
