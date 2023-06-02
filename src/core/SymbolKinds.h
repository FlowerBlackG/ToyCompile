/*
    符号类型

    created on 2022.12.2

*/

#pragma once

#include <unordered_map>
#include <string>

enum class SymbolKind : unsigned int {

#define SYM(X) X,
    #include "core/SymbolKinds.def"
#undef SYM

    NUM_SYMBOLS

};

class SymbolKindUtils {
public:

    static SymbolKindUtils& getInstance();

    std::unordered_map<std::string, SymbolKind> symbolKindMap;
    void makeSymbolKindMap();


private:
    SymbolKindUtils();
    ~SymbolKindUtils();
    SymbolKindUtils(const SymbolKindUtils&) {};

};

