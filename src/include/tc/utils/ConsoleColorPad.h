/*

    created on 2022.12.5
*/

#pragma once

#include <iostream>

class ConsoleColorPad {

public:

    static void setColor(int red, int green, int blue) {
        std::cout << "\e[1;38;2;" << red << ";" << green << ";" << blue << "m";
    }

    static void setColor() {
        std::cout << "\e[0m";
    }

private:


};

