// SPDX-License-Identifier: MulanPSL-2.0

/*

    LR Parser Action Goto Table.
    Part of the ToyCompile Project.

    created on 2022.11.2

*/

#pragma once

#include <vector>
#include <unordered_map>

#include <core/Grammar.h>
#include <iostream>

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

    /**
     * 转移指令。
     * 
     * 例：
     *   s1
     *     s -> type
     *     1 -> target
     * 
     *   r4
     *     r -> type
     *     4 -> target
     */
    struct LrParserCommand {
        /** 指令类型。 */
        LrParserCommandType type;

        /** 转移目标。 */
        int target;
    };

    

    /**
     * LR 文法分析表（Action Goto 表）。
     * 
     * 保存格式：tcpt (ToyCompile Parser Table)
     *   每行一个命令。先以一个词识别指令码，之后根据不同指令码执行操作。
     * 
     * tcpt 命令：
     *   pStId [x]: 设置 primaryStateId。
     *   sym [n] [i] [t] [tk] [sk]: 添加一个 Symbol。
     *                         n -> name, i -> id, t -> type, tk -> tokenKind
     *                         sk -> symbolKind
     *   fe [i] [ti] (rule values...) end: 添加一条表达式。
     *                         i -> id, ti -> targetSymbolId
     * 
     *   tc [r] [c] [ty] [tar]: 添加一条指令。
     *                      r -> row, c -> col, ty -> command type, tar -> target
     */
    struct LrParserTable {

        /**
         * 初始状态 id。
         */
        int primaryStateId;

        /**
         * 符号表。包含文法支持的所有终结符和非终结符。
         * 符号 id 和符号在列表内的下标相同。
         */
        std::vector< grammar::Symbol > symbolList;

        /**
         * 产生式列表。
         */
        std::vector< grammar::FlatExpression > flatExpressions;

        /**
         * Action Goto 表。不推荐直接读取。推荐使用 getCommand 方法获取转移命令。
         */
        std::unordered_map<int, std::unordered_map<int, LrParserCommand>> table;

        /**
         * 获取转移指令。
         * 
         * @param stateId 当前状态 id。
         * @param symbolId 即将遇到的符号的 id。
         * @return 转移指令。对于空白位置，会返回 Error 指令。
         */
        LrParserCommand getCommand(int stateId, int symbolId);

        /**
         * 导出本表，以便后续加载。
         * 导出结果遵循 tcpt 规范。规范详见分析表类定义说明。
         * 
         * @param out 输出流。
         */
        void dump(std::ostream& out);

        /**
         * 从流加载文法分析表。要求输入流内容遵循 tcpt 格式规范。
         * 
         * @param in 输入流。
         * @param msgOut 信息输出流。一般会输出读取过程遇到的问题。
         * @return 加载结果。返回 0 表示加载成功。失败会返回非 0 值。
         */
        int load(std::istream& in, std::ostream& msgOut);
    };

}

