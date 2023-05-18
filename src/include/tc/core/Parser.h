/*

    Parser.h
    part of the ToyCompile project.
    created on 2022.10.31

*/

/*

  ToyCompile Parser 工作原理
      
    Parser 负责分析符号串，并构造语法树。
    符号串由 Lexer 分析得到，作为 Parser 的输入。

    分析过程借助 Action Goto 表完成。
    从表中获取指令，执行对应分析动作。

    Action Goto 表通过原始文法定义文件构建。
    在 ToyCompile 项目中，我们使用魔改版 yacc 规定文法。
    魔改规定详见 tc/core/YaccTcey.h

    通过 YaccTcey 工具加载文法，之后构建成标准文法格式。
    标准文法格式详见 tc/core/grammar 内的 Grammar 结构。

    将 Grammar 输入到 Lr1Grammar 工具内，完成 LR1 文法构建。
    该工具及其原理详见 tc/core/Lr1Grammar。
    
    Lr1Grammar 工具提供一个转移表构建功能，会将其 LR1 文法
    转换成 Action Goto 表。本 Parser 依赖该表格进行分析。

  
  缓存优化（外部设计）

    Parser 并不关心 Action Goto 表的构建过程。它只将外部传递
    的表拷贝到自己内部，并在分析时使用。

    考虑到 Action Goto 表构建较为费时，外部可以考虑一次构建后，
    保存到文件，后续直接加载此前构建完毕的表，以节省启动时间。    

*/

#pragma once

#include <tc/core/AstNode.h>
#include <tc/core/Grammar.h>
#include <tc/core/LrParserTable.h>
#include <vector>
#include <string>

namespace tc {

    /**
     * 文法分析器报错信息。
     */
    struct ParserParseError {
        /**
         * 是否与 token 有关。
         */
        bool tokenRelated;

        /**
         * 错误相关 token。仅当 tokenRelated = true 时有效。
         */
        Token token;

        /**
         * 错误备注信息。
         */
        std::string msg;
    };

    /**
     * 语法分析器。基于 LR 语法表，识别输入串，构建语法树。
     */
    class Parser {
    public:

        Parser();
        explicit Parser(const LrParserTable& parserTable);

        /**
         * 加载 Action Goto 表。会将输入的表复制一份到 parser 内。
         * 
         * @param parserTable Action Goto 表。
         */
        void loadParserTable(const LrParserTable& parserTable);

        /**
         * 根据输入的符号列表，构建语法树。
         * 
         * @param tokens 符号表。结尾需要是 eof 符号。
         * @param errorList 语法错误列表。
         * @return 错误数量。为 0 表示没有遇到语法错误。
         */
        int parse(
            std::vector< Token >& tokens,
            std::vector< ParserParseError >& errorList
        );

        /**
         * 清理。会释放语法树。
         */
        void clear();

        ~Parser();

    public: // getters
        AstNode* getAstRoot() { return this->astRoot; }

    protected:
    
        /**
         * 语法树根节点。
         * Parser 会保持对该节点的管控。
         * 当 Parser 生命周期结束时，会同步释放整个语法树。
         */
        AstNode* astRoot = nullptr;

        /**
         * Action Goto 表。
         */
        LrParserTable parserTable;

    private:

        Parser(const Parser&) {};

    };

}
