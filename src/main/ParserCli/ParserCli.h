// SPDX-License-Identifier: MulanPSL-2.0

/*

    语法分析器。
    创建：2022.11.1

*/

#pragma once

#include <core/config.h>
#include <main/TcSubProgram.h>

class ParserCli : public TcSubProgram {

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
