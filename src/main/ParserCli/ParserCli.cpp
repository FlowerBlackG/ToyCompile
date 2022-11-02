/*

    语法分析器。
    创建：2022.11.1

*/

#include <tc/main/ParserCli/ParserCli.h>
#include <tc/core/YaccTcey.h>

#include <iostream>

using namespace std;
using namespace tc;

void ParserCli::printUsage(std::ostream &out) {
    out << "Parser CommandLine." << endl;

}

int ParserCli::run(
    std::map<std::string, std::string>& paramMap,
    std::set<std::string>& paramSet,
    std::vector<std::string>& additionalValues,
    std::istream& in,
    std::ostream& out
) {

    YaccTcey yacc("resources/ansi-c.tcey.yacc");
    if (yacc.errcode != YaccTceyError::TCEY_OK) {
        out << "[error] ";
        out << yacc.errmsg << endl;
        return -1;
    }

    

    return 0;

}
