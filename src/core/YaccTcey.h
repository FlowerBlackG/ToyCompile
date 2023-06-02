/*

    Yacc ToyCompile Extended Yacc 
    part of the ToyCompile project.
    
    created on 2022.10.31

*/

/*

    tcey 规则：
    在 yacc 语法规范上，额外加入“可解析注释”。

    以 [/ * _tcey_] 开头，[* /] 结尾（方括号内为关键内容。无空格）。
    其中，每三个词句为一组，以空白符号分隔，用于定义终结符内容。
    
    三词组合格式：
      token-key ${key} ${value}

    其中，${key} 为 yacc 语法内的终结符。如c语言的 DOUBLE。
      ${value} 为该终结符字符串（无前后引号）。如：double

    例：
      token-key DOUBLE double
      token-key EXTERN extern
      token-key '%' %

    限制：
      其他多行注释开始符号后至少跟随 1 个空白符号。
      不支持单行注释。
      “可解析注释”开始符号后至少跟随 1 个空白符好。

*/

#pragma once

#include <string>
#include <istream>
#include <vector>
#include <map>

#include <core/TokenKinds.h>
#include <core/Grammar.h>

namespace tc {

    enum class YaccTceyError {
        NOT_INITIALIZED,
        TCEY_OK,
        CANT_OPEN_GRAMMAR_FILE,

        BAD_EXPRESSION_FOUND,
        UNKNOWN_SYMBOL_FOUND,

    };

    /**
     * Yacc Tcey 文法。
     * 
     * 读取 yacc tcey 格式文法文件，并转换为机内表示形式。
     * 文法定义文件解析成功后，文法将被存储在 expressions 内。
     * 
     * expressions 内的每个元素表示一个产生式，形如 A -> xxx | xxx | ...
     * 详见 YaccTceyExpression 结构。
     */
    class YaccTcey {
    public:
        YaccTcey();
        YaccTcey(std::istream& grammarIn);
        YaccTcey(const std::string& grammarPath);

        /**
         * 加载文法。
         * 
         * @param grammarPath 文法文件路径。
         * @return YaccTceyError 分析器状态。
         */
        YaccTceyError loadGrammar(const std::string& grammarPath);

    public:
        YaccTceyError errcode = YaccTceyError::NOT_INITIALIZED;
        std::string errmsg = "";

    public:
        /**
         * 文法。
         */
        grammar::Grammar grammar;

    protected:
        /**
         * 加载文法。内部调用。
         * 
         * @param grammarIn 打开的输入流。
         * @return YaccTceyError 分析器状态。
         */
        YaccTceyError loadGrammar(std::istream& grammarIn);

    protected:
        YaccTceyError loadGrammarBody(std::istream& in);
        grammar::Symbol nameToGrammarSymbol(const std::string& symbolName);
        grammar::Symbol getSymbolById(int id);

        /**
         * 加载表达式区域。该区域被两组 %% 符号包裹。
         * 
         * @return 是否读取成功。
         *         若遇到异常结尾，或遇到严重错误，会返回 false。
         *         若成功读取一个表达式， 会返回 true。
         */
        bool loadExpression(std::istream& in);

        /**
         * 加载 tcey 拓展定义区域。
         */
        YaccTceyError loadTceyBlock(std::istream& in);

    protected:
        std::unordered_map<std::string, TokenKind> tokenKeyKindMap;

        /**
         * 符号映射表。从符号的字符串名，映射到结构。
         */
        std::unordered_map<std::string, grammar::Symbol> symbolMap;

        /**
         * 符号表。符号 id 为其在表内的下标。
         * 即：symbolList[x].id = x 
         */
        std::vector<grammar::Symbol>& symbolList = grammar.symbols;

    private:
        YaccTcey(const YaccTcey&) {};

    };


}
