/*

    Yacc ToyCompile Extended Yacc 
    part of the ToyCompile project.
    
    created on 2022.10.31

*/

#include <tc/core/YaccTcey.h>
#include <fstream>

#include <tc/core/config.h>

using namespace std;
using namespace tc;

/* ------------ public ------------ */

YaccTcey::YaccTcey() {
    this->loadGrammar(TC_CORE_CFG_PARSER_C_TCEY_PATH);
}

YaccTcey::YaccTcey(istream& grammarIn) {
    this->loadGrammar(grammarIn);
}

YaccTcey::YaccTcey(const string& grammarPath) {
    this->loadGrammar(grammarPath);
}

YaccTceyError YaccTcey::loadGrammar(const string& grammarPath) {
    ifstream fin(grammarPath, ios::binary);
    if (fin.is_open()) {
        return this->loadGrammar(fin);
    } else {
        this->errcode = YaccTceyError::CANT_OPEN_GRAMMAR_FILE;
        this->errmsg = "failed to open: ";
        this->errmsg += grammarPath;
        return this->errcode;
    }
}

/* ------------ internal ------------ */

static void __tceyIgnoreLine(istream& in) {
    int ch;
    while (ch = in.peek()) {
        if (ch == EOF) {
            break;
        } 
        
        in.get();

        if (ch == '\n') {
            break;
        }
    }
}

static void __tceyIgnoreMultilineComment(istream& in) {

    while (true) {
        int ch = in.peek();
        if (ch == EOF) {
            break;
        }

        in.get();

        if (ch == '*') {
            ch = in.peek();
            if (ch == '/') {
                in.get();
                break;
            }
        }
    }

}


/* ------------ protected ------------ */

YaccTceyError YaccTcey::loadGrammar(istream& grammarIn) {
    auto& in = grammarIn;
    string keyword;

    this->errcode = YaccTceyError::NOT_INITIALIZED;
    this->tokenKeyKindMap.clear();
    this->grammar.expressions.clear();
    this->symbolMap.clear();
    this->symbolList.clear();

    while (true) {

        if (this->errcode != YaccTceyError::NOT_INITIALIZED) {
            break;
        }

        // 判断流状态是否正常。
        if (in.bad() || in.fail() || !in.good()) {
            break;
        }

        // 判断文件是否结束。
        if (in.peek() == EOF) {
            break;
        }

        in >> keyword;
        if (keyword == "%start") {
            
            string entryName;
            in >> entryName;
            this->grammar.entryId = this->nameToGrammarSymbol(entryName).id;

        } else if (keyword == "%%") {
            
            this->loadGrammarBody(in);
            break;
       
        } else if (keyword[0] == '%') {
       
            __tceyIgnoreLine(in);
       
        } else if (keyword == "/*") {
       
            __tceyIgnoreMultilineComment(in);
       
        } else if (keyword == "/*_tcey_") {
       
            this->loadTceyBlock(in);
       
        } else {
            // 该行内容看不懂。直接跳过。
            __tceyIgnoreLine(in);
        }
    }

    if (this->errcode == YaccTceyError::NOT_INITIALIZED) {
        this->errcode = YaccTceyError::TCEY_OK;
    }

    return this->errcode;
}


YaccTceyError YaccTcey::loadGrammarBody(istream& in) {
    string keyword;

    while (true) {
        if (this->errcode != YaccTceyError::NOT_INITIALIZED) {
            break;
        }

        // 判断流状态是否正常。
        if (in.bad() || in.fail() || !in.good()) {
            break;
        }

        // 判断文件是否结束。
        if (in.peek() == EOF) {
            break;
        }

        if (!this->loadExpression(in)) {
            break;
        }
    }

    return this->errcode;
}

grammar::Symbol YaccTcey::nameToGrammarSymbol(const string& symbolName) {
    if (this->symbolMap.count(symbolName)) {
        return symbolMap[symbolName];
    }

    // 找不到呜呜呜... 创建一个吧。
    this->symbolList.emplace_back();
    auto& symbol = this->symbolList.back();
    symbol.id = this->symbolList.size() - 1;
    symbol.name = symbolName;
    if (symbolName[0] >= 'a' && symbolName[0] <= 'z') {
        // 非终结符。
        symbol.type = grammar::SymbolType::NON_TERMINAL;
    } else {
        // 终结符。
        symbol.type = grammar::SymbolType::TERMINAL;

        if (this->tokenKeyKindMap.count(symbolName)) {
            symbol.tokenKind = this->tokenKeyKindMap[symbolName];
        } else {
            
            symbol.tokenKind = TokenKind::unknown;
            this->errcode = YaccTceyError::UNKNOWN_SYMBOL_FOUND;
            this->errmsg = "unrecognized symbol found: ";
            this->errmsg += symbolName;

        }
    }

    this->symbolMap[symbolName] = symbol;
    return symbol;
}

grammar::Symbol YaccTcey::getSymbolById(int id) {
    if (id > symbolList.size() || id < 1) {
        grammar::Symbol res;
        res.id = -1;
        return res;
    } else {
        return symbolList[id - 1];
    }
}

bool YaccTcey::loadExpression(istream& in) {

    string targetSymbolName;
    string keyword;

    /*
    
        conditional_expression
	        : logical_or_expression
	        | logical_or_expression '?' expression ':' conditional_expression
	        ;
    
    */

    // 读取目标符号。
    in >> targetSymbolName;

    // 判断是否遇到结尾，或流状态异常。
    if (targetSymbolName == "%%" || !in.good() || in.fail() || in.peek() == EOF) {
        return false;
    }

    // 执行到这里，说明没有到达结尾。
    // 之后，应该遇到一个冒号。
    in >> keyword;
    
    if (keyword != ":") {
        this->errcode = YaccTceyError::BAD_EXPRESSION_FOUND;
        this->errmsg = "failed to load Yacc Tcey expression. ':' expected. ";
        this->errmsg += "symbol name before it was: ";
        this->errmsg += targetSymbolName;
        this->errmsg += ". keyword read: ";
        this->errmsg += keyword;
        this->errmsg += ". stream pos is (after keyword): ";
        this->errmsg += to_string(in.tellg());
        return false;
    }

    this->grammar.expressions.emplace_back();
    auto& expression = this->grammar.expressions.back();
    expression.id = this->grammar.expressions.size() - 1;
    expression.targetSymbolId = this->nameToGrammarSymbol(targetSymbolName).id;

    auto& rules = expression.rules;

    // A -> ABC | BC
    // 其中，ABC 是一个 rule，BC 也是一个 rule.
    vector<int> rule;

    // 循环读取，把表达式吃进来。
    while (true) {

        // 如果直接杀到文件结尾了，意味着肯定出现了什么问题...
        if (in.peek() == EOF) {
            this->errcode = YaccTceyError::BAD_EXPRESSION_FOUND;
            this->errmsg = "failed to load Yacc Tcey expression (unexpected eof).";
            return false;
        }

        in >> keyword;
        if (keyword == ";" || keyword == "|") {
            
            // 单个 andExpression 结束。
            if (!rule.empty()) {
                rules.push_back(rule);
                rule.clear();
            }

            if (keyword == ";") {
                break;
            }

        } else {

            rule.push_back(this->nameToGrammarSymbol(keyword).id);

        }

    }

    return true;
}

YaccTceyError YaccTcey::loadTceyBlock(istream& in) {
    string keyword;
    string key;
    string value;

    auto& tokenKindMap = TokenKindUtils::getInstance().tokenKindMap;

    while (true) {
        // 判断流状态是否正常。
        if (in.bad() || in.fail() || !in.good()) {
            break;
        }

        // 判断文件是否结束。
        if (in.peek() == EOF) {
            break;
        }

        in >> keyword;

        if (keyword == "*/") {
        
            break;
        
        } else if (keyword == "token-key") {

            in >> key >> value;
            
            if (tokenKindMap.count(value)) {
                this->tokenKeyKindMap[key] = tokenKindMap[value];
            }

        } else {

            __tceyIgnoreLine(in);
        
        }


    }

    return this->errcode;
}
