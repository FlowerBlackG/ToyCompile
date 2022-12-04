
#include <tc/core/tcir/ValueType.h>
#include <cstdlib>

using namespace tc::tcir;

int ValueTypeUtils::getBytes(const ValueType& type) {
    switch (type) {
        case ValueType::s8:
        case ValueType::u8: {
            return 1;
        }

        
        case ValueType::s16:
        case ValueType::u16: {
            return 2;
        }

        
        case ValueType::s32:
        case ValueType::u32: {
            return 4;
        }

        case ValueType::type_void: {
            return 0;
        }
    
    }

    exit(0x23456); // 不应到达这行代码。
}

bool ValueTypeUtils::isSigned(const ValueType& type) {
    switch (type) {
        case ValueType::s8:
        case ValueType::s16:
        case ValueType::s32: {
            return true;
        }

        
        case ValueType::u8:
        case ValueType::u16:
        case ValueType::u32: {
            return false;
        }

        case ValueType::type_void: {
            return false;
        }
    }

    exit(0x9876); // 不应到达这行代码。
}


const char* ValueTypeUtils::getName(const ValueType& type) {
    if (type == ValueType::s8) {

        return "s8";

    } else if (type == ValueType::s16) {

        return "s16";

    } else if (type == ValueType::s32) {

        return "s32";

    } else if (type == ValueType::u8) {

        return "u8";

    } else if (type == ValueType::u16) {

        return "u16";

    } else if (type == ValueType::u32) {

        return "u32";

    } else if (type == ValueType::type_void) {

        return "void";

    }

    exit(0xabc);
}
