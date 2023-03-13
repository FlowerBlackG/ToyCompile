/*

    i386 汇编代码生成。

    创建：2023年3月9日。

*/


#include <tc/core/Intel386AssemblyGenerator.h>
#include <tc/core/tcir/IrContainer.h>

#include <tc/utils/ConsoleColorPad.h>

using namespace std;
using namespace tc;

namespace tc::i386 {

/**
 * 处理 tcir 的 extlink 区段。
 * extlink 与后续代码生成过程都是分离的。不需要返回解析的结果。
 * 
 * @param in 输入流。
 * @param out 汇编代码输出流。
 * @param err 错误信息输出流。
 */
static void __processExtLink(
    istream& in,
    ostream& out,
    ostream& err
) {
    string token;
    
    while (true) {
        in >> token;
        if (token == "@") {
            in >> token >> token >> token;
            break;
        } else if (token == "export") {
            // export ${symbol} ${"fun" | "var"}

            string type;
            in >> token >> type;

            out << "global " << token << endl;

        } else if (token == "import") {
            // import ${symbol}
            in >> token;
            out << "extern " << token << endl;
        }
    }
}

static bool __extractInstructionCodeFromStream(
    istream& in,
    tcir::IrInstructionCode& container
) {

    const auto isNewLine = [] (int ch) {
        return ch == '\r' || ch == '\n';
    };

    const auto isSpaceOrTab = [] (int ch) {
        return ch == ' ' || ch == '\t';
    };

    const auto isBlank = [&isNewLine, &isSpaceOrTab] (int ch) {
        return isSpaceOrTab(ch) || isNewLine(ch);
    };

    // 过滤前置空白字符（含换行）。
    while (isBlank(in.peek())) {
        in.get();
    }

    while (true) {

        // 过滤前置空白字符。
        while (isSpaceOrTab(in.peek())) {
            in.get();
        }

        if (in.peek() == '@') {
            return false; // 到 instruction 的结尾了。
        } else if (isNewLine(in.peek())) {
            // 要换行了。
            break;
        }

        string token;
        while (!isBlank(in.peek())) {
            token.push_back(in.get());
        }
        container.push_back(token);
    }

    return true;
}



static void __optimizeInstructions(
    vector<tcir::IrInstructionCode>& instructions
) {
    for (size_t idx = 0; idx < instructions.size(); idx++) {
        auto& curr = instructions[idx];
        if (idx + 1 < instructions.size()) {
            auto& next = instructions[idx + 1];


#define REMOVE_NEXT_AND_CONTINUE() \
    instructions.erase(instructions.begin() + idx + 1); \
    idx --; \
    continue;

#define REMOVE_CURR_AND_CONTINUE() \
    instructions.erase(instructions.begin() + idx); \
    idx --; \
    continue;


            // 删除连续的两个 ret 符号。
            if (curr.isRet() && next.isRet()) {
                REMOVE_NEXT_AND_CONTINUE();
            }

            // 删除成对出现的 push pop。
            // 注意，实际情况下对于 eflags 的成对 pop push 不宜删除。
            if (curr.isPairedPushPopWith(next)) {
                REMOVE_NEXT_AND_CONTINUE();
            }

            if (curr == next && curr.isMov()) {
                REMOVE_NEXT_AND_CONTINUE();
            }

            // 对于重复 mov 到同一个目标的行为，保留下一条指令。
            if (curr.isMovToSameTargetWith(next)) {
                REMOVE_CURR_AND_CONTINUE();
            }

            // 循环赋值，删掉后面那句。
            if (curr.isCircularMovWith(next)) {
                REMOVE_NEXT_AND_CONTINUE();
            }


#undef REMOVE_NEXT_AND_CONTINUE
#undef REMOVE_CURR_AND_CONTINUE

            // 三步展望优化。
            if (idx + 2 < instructions.size()) {
                auto& nextNext = instructions[idx + 2];

                if (curr.isPushVreg0() 
                    && nextNext.isPopVreg1() 
                    && next.isMovToVreg0()
                ) {
                    curr = "mov vreg 1 vreg 0";
                    instructions.erase(instructions.begin() + idx + 2);
                    idx --;
                    continue;
                }
            }

        }
    }
}

static void __extractInstructionsFromStream(
    istream& in,
    vector<tcir::IrInstructionCode>& container
) {

    tcir::IrInstructionCode code;
    while (__extractInstructionCodeFromStream(in, code)) {
        container.push_back(code);
        code.clear();
    }

    // 删去 instruction 区段的结尾符号。
    string token;
    in >> token >> token >> token >> token;

}

void Intel386AssemblyGenerator::processInstructionsSection(
    istream& in,
    ostream& out,
    ostream& err
) {
    string token;

    vector<tcir::IrInstructionCode> instructions;
    __extractInstructionsFromStream(in, instructions);
    __optimizeInstructions(instructions);

    ConsoleColorPad::setColor(0xee, 0x3f, 0x4d);
    for (auto& it1 : instructions) {
        for (auto& it2 : it1) {
            cout << it2 << ' ';
        }
        cout << endl;
    }
    ConsoleColorPad::setColor();

    buildAssemblyFile(instructions, out, err);

}

void Intel386AssemblyGenerator::buildAssemblyFile(
    vector<tcir::IrInstructionCode>& instructions,
    ostream& out,
    ostream& err
) {

    for (auto& code : instructions) {
        parseCode(code, out, err);
    }

}

void Intel386AssemblyGenerator::parseCode(
    tcir::IrInstructionCode& code,
    ostream& out, 
    ostream& err
) {

    if (code.isRet()) {

        out << "leave" << endl
            << "ret" << endl << endl;

    } else if (code.isLabel()) {

        out << code[1] << ":" << endl;

    } else if (code.isCall()) {

        out << "call " << code[1] << endl;

    } else if (code.isPushForCall()) {

        out << "push ";
        // todo

        out << endl;

    } else if (code.isFunLabel()) {

        out << endl;
        out << code[1] << ":" << endl;
        
        // todo

    }

}


int Intel386AssemblyGenerator::generate(
    istream& in,
    ostream& out,
    ostream& err
) {

    this->clear();
    string token;

    out << "; generated by ToyCompile" << endl;
    out << "; for intel 386 protected mode environment" << endl;
    out << endl;
    out << "[bits 32]" << endl;
    out << "section .text" << endl << endl;

    
    while (true) {
        in >> token;
        if (token != "@") {
            err << "[err] bad token." << endl;
            return -1;
        }
        
        in >> token >> token >> token;

        if (token == "extlink") {

            __processExtLink(in, out, err);

        } else if (token == "static-data") {

            // 暂时不管静态数据区。
            while (true) {
                in >> token;
                if (token == "@") {
                    in >> token >> token >> token;
                    break;
                }
            }

        } else if (token == "global-symtab") {

            tcir::GlobalSymbolTable::build(in, err, globalSymTab);

        } else if (token == "block-symtab") {

            tcir::BlockSymbolTable::build(
                in, err, blockSymTabMap, &varDescTab
            );

        } else if (token == "instructions") {

            this->processInstructionsSection(in, out, err);

            break;
        }
    }

    return 0;

}

void Intel386AssemblyGenerator::clear() {
    this->varDescTab.clear();
    this->globalSymTab.clear();
    
    for (auto it : this->blockSymTabMap) {
        delete it.second;
    }
    this->blockSymTabMap.clear();
}


}
