/*

    i386 汇编代码生成。

    创建：2023年3月9日。

*/


#include <core/Intel386AssemblyGenerator.h>
#include <core/tcir/IrContainer.h>

#include <utils/ConsoleColorPad.h>

#include <functional>

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
                instructions.erase(instructions.begin() + idx);
                instructions.erase(instructions.begin() + idx);
                idx--;
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

    buildAssemblyFile(instructions, out, err);

}

void Intel386AssemblyGenerator::processStaticData(istream& in, ostream& out) {

    string token;
    while (true) {
        in >> token;
        if (token == "@") {
            in >> token >> token >> token;
            break;
        }

        if (token == "int") {
            string access, valName, len, value;
            in >> access >> valName >> len >> value;
            out << "align 4" << endl;
            out << valName << ":" << endl;

            int iLen = 32 / 8; // 硬编码。强制 4 字节。
            uint64_t iValue = stoull(value);

            out << "  db ";
            while (iLen--) {
                out << (iValue % 0xFF);
                if (iLen) {
                    out << ", ";
                }
                iValue >>= 8;
            }
            out << endl;

        }
    }

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

void Intel386AssemblyGenerator::parseVariable(
    string& type, string& name, ostream& out
) {

    if (type == "imm") { // 立即数

        out << name;

    } else if (type == "val") { // 变量

        int valId = -1;
        try {
            valId = stoi(name);
        } catch (...) {}


        if (valId == -1) {
            // 全局变量
            out << "[" << name << "]";
        } else {
            // 局部变量

            // 进入函数时，已经预先分配栈空间（add esp, x）。
            // 因此，栈内存位置应该比 esp 的值大。

            auto offset = this->variableStackOffsetMap[valId] - 4;
            out << "[ebp ";
            if (offset > 0) {
                out << "+ ";
                out << offset;
            } else if (offset == 0) {
                // do nothing
            } else { // offset < 0
                
                out << offset;
            }
            out << "]";
        }

    } else if (type == "vreg") { // 虚拟寄存器

        if (name == "0") {
            out << "eax";
        } else if (name == "1") {
            out << "edx";
        }

    } else if (type == "fval") { // 函数参数

        auto symIdx = this->currentFunction->findParamSymbolIndex(name);
        
        out << "[ebp + ";
        out << symIdx * 4 + 8;
        out << "]";

    }

}

void Intel386AssemblyGenerator::parseCode(
    tcir::IrInstructionCode& code,
    ostream& out, 
    ostream& err
) {

    if (code.isRet()) {
        
        out << "  leave" << endl
            << "  ret" << endl << endl;

    } else if (code.isLabel()) {

        out << code[1] << ":" << endl;

    } else if (code.isCall()) {

        out << "  call " << code[1] << endl;
        
        auto& funcParamList = globalSymTab.getFunction(code[1])->params;
        int restoreStackSize = 0;
        for (auto& it : funcParamList) {
            
            restoreStackSize += tcir::ValueTypeUtils::getBytes(it.valueType);
        }

        if (restoreStackSize) {
            out << "  add esp, " << restoreStackSize << endl;
        }
        

    } else if (code.isPushForCall()) {

        out << "  push ";
        this->parseVariable(code[2], code[3], out);

        out << endl;

    } else if (code.isFunLabel()) {

        auto& funName = code[1];

        out << endl;
        out << funName << ":" << endl;

        out << "  push ebp" << endl;
        out << "  mov ebp, esp" << endl;
        
        auto pFun = globalSymTab.getFunction(funName);

        this->currentFunction = pFun;

        auto funSymTab = this->blockSymTabMap[pFun->rootSymTabId];

        const std::function<
            int (tcir::BlockSymbolTable*)
        > dfs = [&] (tcir::BlockSymbolTable* currTab) {

            int res = 0;

            for (auto it : currTab->children) {
                res = max(res, dfs(it));
            }

            res += currTab->symbols.size() * 4;

            return res;
        };

        this->currFunctionStackMemory = dfs(funSymTab);

        if (this->currFunctionStackMemory) {
            out << "  sub esp, " << this->currFunctionStackMemory << endl;
        }

    } else if (code[0] == "xchg") {
        out << "  xchg ";
        parseVariable(code[1], code[2], out);
        out << ", ";
        parseVariable(code[3], code[4], out);
        out << endl;
    
    } else if (code[0] == "jmp") {

        out << "  jmp " << code[1] << endl;

    } else if (code[0] == "jge") {

        out << "  jge " << code[1] << endl;

    } else if (code[0] == "jg") {

        out << "  jg " << code[1] << endl;

    } else if (code[0] == "je") {

        out << "  je " << code[1] << endl;

    } else if (code[0] == "jl") {

        out << "  jl " << code[1] << endl;

    } else if (code[0] == "jle") {

        out << "  jle " << code[1] << endl;

    } else if (code[0] == "jne") {

        out << "  jne  " << code[1] << endl;

    } else if (code[0] == "push") {
        out << "  push ";
        this->parseVariable(code[2], code[3], out);
        out << endl;
    } else if (code[0] == "pop") {
        out << "  pop ";
        if (code.size() > 2) {
            this->parseVariable(code[2], code[3], out);
        }
        out << endl;
    } else if (code.isMov()) { 

        out << "  mov dword ";
        
        parseVariable(code[1], code[2], out);
        out << ", ";
        parseVariable(code[3], code[4], out);
        out << endl;

    } else if (code[0] == "add") {

        out << "  add dword ";
        parseVariable(code[1], code[2], out);
        out << ", ";
        parseVariable(code[3], code[4], out);
        out << endl;

    } else if (code[0] == "sub") {

        out << "  sub dword ";
        parseVariable(code[1], code[2], out);
        out << ", ";
        parseVariable(code[3], code[4], out);
        out << endl;

    } else if (code[0] == "neg") {
        // todo
    } else if (code[0] == "mul") {

        out << "  imul dword ";
        parseVariable(code[1], code[2], out);
        out << ", ";
        parseVariable(code[3], code[4], out);
        out << endl;

    } else if (code[0] == "and") {
        // todo
    } else if (code[0] == "or") {
        // todo
    } else if (code[0] == "xor") {
        // todo
    } else if (code[0] == "not") {
        // todo
    } else if (code[0] == "cmp") {

        out << "  cmp dword ";
        parseVariable(code[1], code[2], out);
        out << ", ";
        parseVariable(code[3], code[4], out);
        out << endl;

    } 

    // todo

}

void Intel386AssemblyGenerator::buildVariableOffsetMap() {

    /**
     * 深度优先搜索，为每个局部变量分配一个偏移。
     */
    std::function<
        void (tcir::BlockSymbolTable*, int)
    > dfs = [&] (tcir::BlockSymbolTable* curr, int memoryUsed) {

        for (auto it : curr->symbols) {
            this->variableStackOffsetMap[it->id] = memoryUsed;
            memoryUsed += 4; // todo: should be it->bytes;
        }

        for (auto it : curr->children) {
            dfs(it, memoryUsed);
        }
    
    };

    for (auto& it : this->blockSymTabMap) {
        if (it.second == it.second->parent) {
            dfs(it.second, 0);
        }
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

            this->processStaticData(in, out);

        } else if (token == "global-symtab") {

            tcir::GlobalSymbolTable::build(in, err, globalSymTab);

        } else if (token == "block-symtab") {

            tcir::BlockSymbolTable::build(
                in, err, blockSymTabMap, &varDescTab
            );
            
            this->buildVariableOffsetMap();

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
    this->variableStackOffsetMap.clear();
}


}
