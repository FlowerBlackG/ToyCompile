/*

    数据类型。

    created on 2022.12.3
*/

#pragma once

namespace tc::tcir {

    /**
     * 数据类型。
     * u -> unsigned
     * s -> signed
     * uX -> unsigned X_bit 
     */
    enum class ValueType {
        u8,
        u16,
        u32,
        s8,
        s16,
        s32,

        type_void,
    };

    class ValueTypeUtils {
    public:
        static int getBytes(const ValueType& type);
        static bool isSigned(const ValueType& type);
        static const char* getName(const ValueType& type);

    };
}
