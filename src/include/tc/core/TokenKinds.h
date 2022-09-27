/*
 * Token 类别。
 * 创建：2022.9.27
 */

#pragma once


enum class TokenKind : unsigned int {

#define TOK(X) X,
    #include "tc/core/TokenKinds.def"
#undef TOK

    NUM_TOKENS
};
