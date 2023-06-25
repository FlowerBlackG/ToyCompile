// SPDX-License-Identifier: MulanPSL-2.0

/*
 * Token 结构。
 * 创建于 2022年9月27日。
 */

#include <magic_enum/magic_enum.hpp>

#include "core/Token.h"


using namespace std;
using namespace tc;

string_view Token::getKindName() {
    return magic_enum::enum_name(this->kind);
}
