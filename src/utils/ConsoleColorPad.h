// SPDX-License-Identifier: MulanPSL-2.0

/*

    命令行调色板。
    功能：改变文字输出的颜色。

    created on 2022.12.5
*/

#pragma once

#include <iostream>


/**
 * 
 * 命令行调色板。
 * 功能：改变文字输出的颜色。
 * 
 * 如果我们觉得输出的字全都是白花花的，不好看，
 * 可以借助本工具进行调色，实现带颜色的文字输出。
 */
class ConsoleColorPad {

public:

    /**
     * 设置调色板颜色。设置后，后续输出的字符会变成你设置的颜色。
     * 三原色的取值范围都是 [0, 255]。
     */
    static void setColor(int red, int green, int blue) {
        std::cout << "\e[1;38;2;" << red << ";" << green << ";" << blue << "m";
    }

    /**
     * 恢复控制台的默认颜色。
     */
    static void setColor() {
        std::cout << "\e[0m";
    }

private:


};

