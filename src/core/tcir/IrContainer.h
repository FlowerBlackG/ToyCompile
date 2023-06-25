// SPDX-License-Identifier: MulanPSL-2.0

/*

    中间代码容器

    创建于 2023年3月8日 上海市嘉定区安亭镇

*/

#pragma once

#include <vector>
#include <string>

namespace tc::tcir {

/**
 * 单条 IR 指令码。
 *
 *   mov   vreg  0   vreg  1  <- 1条指令。
 *    ^     ^    ^    ^    ^   
 *   [0]   [1]  [2]  [3]  [4]
 * 
 */
class IrInstructionCode : public std::vector<std::string> {

public:
    bool isRet();
    bool isPush();
    bool isPop();
    bool isLabel();
    bool isFunLabel();
    bool isCall();
    bool isPushForCall();
    bool isMov();

    bool isPushVreg0();
    bool isPushVreg1();
    bool isPushVreg();
    bool isPopVreg0();
    bool isPopVreg1();
    bool isPopVreg();

    
    bool isMovToVreg0();

    bool isMovToSameTargetWith(IrInstructionCode& other);
    bool isCircularMovWith(IrInstructionCode& other);

    bool isPairedPushPopWith(IrInstructionCode& other);

    const IrInstructionCode& operator= (const std::string& code);

};




}

