// SPDX-License-Identifier: MulanPSL-2.0

/*
 * Token 结构
 * 创建：2022.9.26
 */

#pragma once

#include <string_view>

#include <core/TokenKinds.h>
#include <utils/CppReflect.h>

namespace tc {

    struct Token {

        /**
         * 符号内容。
         */
        std::string content;
        
        /**
         * 所在列。从1开始索引。
         */
        int col;

        /**
         * 所在行号。从1开始索引。
         */
        int row;

        /**
         * 符号类型。
         */
        TokenKind kind;

        std::string_view getKindName();

    };

}
