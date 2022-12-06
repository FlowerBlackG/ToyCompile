/*

    created on 2022.12.3
*/


#include <tc/core/tcir/SymbolTable.h>

using namespace std;
using namespace tc;


tcir::FunctionParamSymbol* tcir::FunctionSymbol::findParamSymbol(
    const std::string& paramName
) {

    for (auto it : this->params) {
        if (it.name == paramName) {
            return &it;
        }
    }

    return nullptr;

}

tcir::VariableSymbol* tcir::GlobalSymbolTable::getVariable(const string& name) {
    if (this->variables.count(name)) {

        return this->variables[name];
    } else {
        return nullptr;
    }
}

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

tcir::VariableSymbol* tcir::BlockSymbolTable::get(
    const string& name, 
    bool allowFromParents
) {

    if (symbolNameMap.count(name)) {
        return symbolNameMap[name];
    } else if (this->parent == this || !allowFromParents) {
        return nullptr;
    } else {
        return this->parent->get(name, true);
    }

}

tcir::VariableSymbol* tcir::BlockSymbolTable::get(int id, bool allowFromParents) {

    VariableSymbol* res;

    if (symbols.empty()) {

        res = nullptr;

    } else {

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

        res = symbols[left];
    }

    if (res->id == id) {

        return res;

    } else if (this->parent == this || !allowFromParents) {

        return nullptr;

    } else {

        return this->parent->get(id, true);

    }

}



tcir::BlockSymbolTable::~BlockSymbolTable() {
    
}

void tcir::BlockSymbolTable::put(tcir::VariableSymbol* symbol) {
    symbols.push_back(symbol);
    symbolNameMap[symbol->name] = symbol;
    descTable->put(symbol);
}
