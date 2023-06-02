/*

    统一命令行客户端。

    created on 2022.12.7

*/

#pragma once

#include <core/config.h>
#include <main/TcSubProgram.h>

#include <vector>

#include <core/Token.h>
#include <core/Parser.h>
#include <core/tcir/IrGenerator.h>

/**
 * 统一命令行客户端。完成从文件输入到汇编生成的全流程。
 * 通过命令行参数控制中间结果输出。
 * 
 * @todo 目前实现到中间代码生成。汇编将在课程设计内完成。
 */
class UniCli : public TcSubProgram {
public:

    void printUsage(std::ostream& out) override;

    void dumpTokens(
        std::vector<tc::Token>& tkList, 
        std::ostream& out,
        bool disableColor = false
    );

    int lexicalAnalysis(
        std::map<std::string, std::string>& paramMap,
        std::set<std::string>& paramSet,
        std::vector<std::string>& additionalValues,
        std::ostream& logOutput,
        std::vector<tc::Token>& tokenContainer
    );

    int syntaxAnalysis(
        std::map<std::string, std::string>& paramMap,
        std::set<std::string>& paramSet,
        std::vector<std::string>& additionalValues,
        std::ostream& logOutput,
        std::vector<tc::Token>& tokens,
        tc::Parser& parser
    );

    int generateTcIr(
        std::map<std::string, std::string>& paramMap,
        std::set<std::string>& paramSet,
        std::vector<std::string>& additionalValues,
        std::ostream& logOutput,
        tc::AstNode* astRoot,
        tc::tcir::IrGenerator& irGenerator
    );

    int run(
        std::map<std::string, std::string>& paramMap,
        std::set<std::string>& paramSet,
        std::vector<std::string>& additionalValues,
        std::istream& in,
        std::ostream& out
    ) override;

    
protected:
    /** 是否启用控制台颜色输出。 */
    bool enableOutputColor = true;
    
    void setOutputColor(int red, int green, int blue);
    void setOutputColor();

protected:

};
