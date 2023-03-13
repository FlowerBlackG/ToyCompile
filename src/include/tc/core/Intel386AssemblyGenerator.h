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

protected:
    tcir::VariableDescriptionTable varDescTab;
    tcir::GlobalSymbolTable globalSymTab;
    std::map<int, tcir::BlockSymbolTable*> blockSymTabMap;

};

}
