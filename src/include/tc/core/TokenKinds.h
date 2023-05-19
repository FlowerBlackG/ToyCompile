/*
 * Token 类别。
 * 创建：2022.9.27
 */

#pragma once

#include <unordered_map>
#include <string>

enum class TokenKind : unsigned int {

#define TOK(X) X,
    #include "tc/core/TokenKinds.def"
#undef TOK

    NUM_TOKENS
};

/**
 * Token 类别工具包。内含一个 token 类别映射表。
 * 通过单例获取。非必要不修改该映射表。
 */
class TokenKindUtils {
public:
    
    static TokenKindUtils& getInstance();

    /**
     * 类型映射表。
     * 键为字符串，值为对应枚举值。
     * 例：
     *   ?      -> TokenKind::question
     *   inline -> TokenKind::kw_inline
     * 
     * 特殊键：
     *   --_identifier_ 
     *   --_unknown_ 
     *   --_numeric_constant_ 
     *   --_char_constant_
     *   --_string_literal_ 
     *   --_eof_ 
     *   --_single_line_comment_ 
     *   --_multi_line_comment_"
     */
    std::unordered_map<std::string, TokenKind> tokenKindMap;

    void makeTokenKindMap();

private:
    TokenKindUtils();
    ~TokenKindUtils() = default;
    TokenKindUtils(const TokenKindUtils&) {};
};
