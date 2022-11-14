/*
 * 词法分析器服务器。
 * 创建于 2022年9月26日
 */

#pragma once

#include <tc/core/config.h>
#include <tc/main/TcSubProgram.h>

class LexerServer : public TcSubProgram {

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