/*
 * C++ 编译期反射。
 * 创建：2022.9.27
 * 
 * 参考：
 *   https://zhuanlan.zhihu.com/p/419673631
 */

#pragma once

#include <string_view>

template <typename T, T V>
constexpr std::string_view getEnumClassValueName() {
    
    const char* info = __PRETTY_FUNCTION__;

    const char* p = info;
    while (*p != ';') {
        p++;
    }

    while (*p != '=') {
        p++;
    }

    p += 2;

    const char* pTail = p;
    while (*pTail != ';') {
        pTail++;
    }

    std::string_view view(p, pTail - p);

    return view;
}
