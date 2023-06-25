// SPDX-License-Identifier: MulanPSL-2.0

/*

    中间代码容器

    创建于 2023年3月8日 上海市嘉定区安亭镇

*/

#include <core/tcir/IrContainer.h>

using namespace tc;
using namespace std;

namespace tc::tcir {

bool IrInstructionCode::isRet() {
    return size() == 1 && front() == "ret";
}

bool IrInstructionCode::isPush() {
    return front() == "push";
}

bool IrInstructionCode::isPop() {
    return front() == "pop";
}

bool IrInstructionCode::isLabel() {
    return front() == "label";
}

bool IrInstructionCode::isFunLabel() {
    return front() == "fun-label";
}

bool IrInstructionCode::isCall() {
    return front() == "call";
}

bool IrInstructionCode::isMov() {
    return front() == "mov";
}

bool IrInstructionCode::isPushForCall() {
    return front() == "pushfc";
}

bool IrInstructionCode::isPushVreg0() {
    return isPushVreg() && (*this)[3] == "0";
}

bool IrInstructionCode::isPushVreg1() {
    return isPushVreg() && (*this)[3] == "1";
}

bool IrInstructionCode::isPushVreg() {
    return isPush() && (*this)[2] == "vreg";
}

bool IrInstructionCode::isPopVreg0() {
    return isPopVreg() && (*this)[3] == "0";
}

bool IrInstructionCode::isPopVreg1() {
    return isPopVreg() && (*this)[3] == "1";
}

bool IrInstructionCode::isPopVreg() {
    return isPop() && (*this)[2] == "vreg";
}

bool IrInstructionCode::isMovToVreg0() {
    return this->isMov() && (*this)[1] == "vreg" && (*this)[2] == "0";
}

bool IrInstructionCode::isMovToSameTargetWith(IrInstructionCode& other) {
    return this->isMov() && other.isMov() 
        && (*this)[1] == other[1] && (*this)[2] == other[2];
}

bool IrInstructionCode::isCircularMovWith(IrInstructionCode& other) {
    return this->isMov() && other.isMov()
        && (*this)[1] == other[3] && (*this)[2] == other[4]
        && (*this)[3] == other[1] && (*this)[4] == other[2];
}

bool IrInstructionCode::isPairedPushPopWith(IrInstructionCode& other) {
    bool thisIsPop;
    if (this->isPush() && other.isPop()) {
        thisIsPop = false;
    } else if (this->isPop() && other.isPush()) {
        thisIsPop = true;
    } else {
        return false;
    }

    front() = thisIsPop ? "push" : "pop";
    bool res = *this == other;
    front() = thisIsPop ? "pop" : "push";
    return res;
}


const IrInstructionCode& IrInstructionCode::operator= (const string& code) {

    string codeCopy = code;

    this->clear();

    auto isBlank = [] (const char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    };

    while (codeCopy.length() > 0 && isBlank(codeCopy.back())) {
        codeCopy.pop_back();
    }

    if (codeCopy.length() == 0) {
        return *this;
    }

    string codeSegment;

    int currPos = 0;

    // code: "  xxxx   xxxx   xxxx"  <-- 结尾没有空格。
    while (currPos < codeCopy.length()) {
        char c = codeCopy[currPos];

        while (isBlank(c)) {
            c = codeCopy[++currPos];
        }

        codeSegment.clear();

        while (true) {
            codeSegment += c;

            if ((++currPos) >= codeCopy.length()) {
                break;
            }

            c = codeCopy[currPos];
            
            if (isBlank(c)) {
                break;
            }
        }

        this->push_back(codeSegment);
    }

    return *this;
}

}
