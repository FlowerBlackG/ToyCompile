/*
 * Lexer 本地交互工具。
 * 创建：2022年9月28日。
 */

#pragma once

#include <main/TcSubProgram.h>

class LexerCli : public TcSubProgram {

public:

    void printUsage(std::ostream& out) override;

    int run(
        std::map<std::string, std::string>& paramMap,
        std::set<std::string>& paramSet,
        std::vector<std::string>& additionalValues,
        std::istream& in,
        std::ostream& out
    ) override;


protected:

};
