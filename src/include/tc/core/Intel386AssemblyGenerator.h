/*

    i386 汇编代码生成。

    创建：2023年3月9日。

*/

#pragma once

#include <iostream>
#include <map>

#include <tc/core/tcir/SymbolTable.h>
#include <tc/core/tcir/IrContainer.h>

namespace tc::i386 {

class Intel386AssemblyGenerator {

public:

    Intel386AssemblyGenerator() {}

    int generate(
        std::istream& in,
        std::ostream& out,
        std::ostream& err
    );

    void clear();


protected:
    void processInstructionsSection(
        std::istream& in,
        std::ostream& out,
        std::ostream& err
    );

    void processStaticData(std::istream& in, std::ostream& out);

    void buildAssemblyFile(
        std::vector<tcir::IrInstructionCode>& instructions,
        std::ostream& out,
        std::ostream& err
    );

    void parseCode(
        tcir::IrInstructionCode& code,
        std::ostream& out,
        std::ostream& err
    );

    void parseVariable(std::string& type, std::string& name, std::ostream& out);

    void buildVariableOffsetMap();

protected:
    tcir::VariableDescriptionTable varDescTab;
    tcir::GlobalSymbolTable globalSymTab;

    /**
     * 所有的块符号表。
     * 容器 key 为符号表 id，value 为符号表本身。
     */
    std::map<int, tcir::BlockSymbolTable*> blockSymTabMap;

    /**
     * 局部变量偏移表。
     * key：变量 id
     * value：变量的偏移量。
     *        考虑到栈向低地址生长，这个偏移是负值。
     *        如果汇编生成器在进入函数的地方提前减小 sp 的值，则取变量的偏移应该是正数。
     */
    std::map<int, int> variableStackOffsetMap;

protected:

    tcir::FunctionSymbol* currentFunction;
    int currFunctionStackMemory;

};

}
