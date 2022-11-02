/*

    LR Parser Action Goto Table.
    Part of the ToyCompile Project.

    created on 2022.11.2

*/

#pragma once

#include <unordered_map>

namespace tc {

    enum class LrParserCommandType {
        /** 接受。 */
        ACCEPT,

        /** 语法错误。 */
        ERROR,

        /** 状态转移。 */
        GOTO,

        /** 移进。 */
        SHIFT,

        /** 归约。 */
        REDUCE
    };

    struct LrParserCommand {
        LrParserCommandType type;
        int target;
    };

    struct LrParserTable {
        int primaryStateId;
        std::unordered_map<int, std::unordered_map<int, LrParserCommand>> table;

        LrParserCommand getCommand(int stateId, int symbolId);
    };

}
