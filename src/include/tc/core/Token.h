
/*
 * Token 结构
 * 创建：2022.9.26
 */

#pragma once

#include <string>

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
    int line;

};
