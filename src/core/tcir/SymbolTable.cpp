/*

    created on 2022.12.3
*/


#include <tc/core/tcir/SymbolTable.h>

using namespace std;
using namespace tc;

void tcir::GlobalSymbolTable::clear() {
    for (auto it : functions) {
        if (it.second) {
            delete it.second;
        }
    }

    functions.clear();

    for (auto it : variables) {
        if (it.second) {
            delete it.second;
        }
    }

    variables.clear();
}

tcir::GlobalSymbolTable::~GlobalSymbolTable() {
    this->clear();

}


void tcir::VariableDescriptionTable::put(tcir::VariableSymbol* symbol) {
    symbolMap[symbol->id] = symbol;
}

tcir::VariableSymbol* tcir::VariableDescriptionTable::get(int id) {
    if (symbolMap.count(id)) {
        return symbolMap[id];
    } else {
        return nullptr;
    }
}

void tcir::VariableDescriptionTable::clear() {
    for (auto it : symbolMap) {
        delete it.second;
    }

    symbolMap.clear();
}

tcir::VariableDescriptionTable::~VariableDescriptionTable() {
    this->clear();
}

tcir::VariableSymbol* tcir::BlockSymbolTable::get(const string& name) {

    if (symbolNameMap.count(name)) {
        return symbolNameMap[name];
    } else {
        return nullptr;
    }

}

tcir::VariableSymbol* tcir::BlockSymbolTable::get(int id) {

    if (symbols.empty()) {
        return nullptr;
    }

    int left = 0;
    int right = symbols.size() - 1;

    while (left < right) {

        int mid = (left + right) / 2;
        
        if (id <= symbols[mid]->id) {

            right = mid;
        
        } else {
        
            left = mid + 1;
        
        }
    }

    auto symbolFound = symbols[left];

    return symbolFound->id == id ? symbolFound : nullptr;

}



tcir::BlockSymbolTable::~BlockSymbolTable() {
    
}

void tcir::BlockSymbolTable::create(tcir::VariableSymbol* symbol) {
    symbols.push_back(symbol);
    symbolNameMap[symbol->name] = symbol;
    descTable->put(symbol);
}
