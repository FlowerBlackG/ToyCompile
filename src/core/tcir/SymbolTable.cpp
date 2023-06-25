// SPDX-License-Identifier: MulanPSL-2.0

/*

    created on 2022.12.3
*/


#include <core/tcir/SymbolTable.h>
#include <iostream>

using namespace std;
using namespace tc;

tcir::FunctionParamSymbol* tcir::FunctionSymbol::findParamSymbol(
    const std::string& paramName
) {

    int idx = this->findParamSymbolIndex(paramName);

    if (idx == -1) {
        return nullptr;
    } else {
        return &this->params[idx];
    }

}

int tcir::FunctionSymbol::findParamSymbolIndex(
    const std::string& paramName
) {

    for (int i = 0; i < this->params.size(); i++) {

        if (this->params[i].name == paramName) {
            return i;
        }
    }

    return -1;

}

tcir::FunctionSymbol* tcir::GlobalSymbolTable::getFunction(const string& name) {
    if (this->functions.count(name)) {
        return this->functions[name];
    } else {
        return nullptr;
    }
}

tcir::VariableSymbol* tcir::GlobalSymbolTable::getVariable(const string& name) {
    if (this->variables.count(name)) {

        return this->variables[name];
    } else {
        return nullptr;
    }
}


void tcir::GlobalSymbolTable::put(FunctionSymbol* func) {
    this->functions[func->name] = func;
}

void tcir::GlobalSymbolTable::put(VariableSymbol* vari) {
    this->variables[vari->name] = vari;
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

int tcir::GlobalSymbolTable::build(
    std::istream& in, 
    std::ostream& err,
    GlobalSymbolTable& container
) {

    string token;

    while (true) {
        in >> token;
        
        if (token == "@") {
            in >> token >> token >> token;
            break;
        } else if (token == "fun") {

            string visi, name, argc, retType, rootSymTabId;
            in >> visi >> name >> argc >> retType >> rootSymTabId;

            auto pFun = new FunctionSymbol;
            pFun->name = name;
            pFun->isImported = false;
            pFun->visibility = SymbolVisibility::global;
            pFun->symbolType = SymbolType::functionDefine;
            pFun->rootSymTabId = stoi(rootSymTabId);

            container.put(pFun);

            // 处理参数表。

            int i32argc = stoi(argc);

            for (int i = 0; i < i32argc; i++) {
                string type, valueOrPtr, name; 
                in >> type >> valueOrPtr ;
                
                if (type == "void") {
                    continue;
                }
                
                in >> name;

                auto& pParamSym = pFun->params.emplace_back();
                pParamSym.isPointer = false;
                pParamSym.isVaList = false;
                pParamSym.name = name;
                pParamSym.symbolType = SymbolType::functionParam;
                pParamSym.valueType = ValueType::s32;
                pParamSym.visibility = SymbolVisibility::internal;
            }

        } else if (token == "var") {

            string name, type, bytes;
            in >> name >> type >> bytes;
            auto pVar = new VariableSymbol;

            pVar->name = name;
            pVar->symbolType = SymbolType::variableDefine;
            pVar->valueType = ValueType::s32;
            pVar->bytes = stoi(bytes);

            container.put(pVar);

        }
    }

    return 0;
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

void tcir::BlockSymbolTable::dump(ostream& out) {
    out << "% begin" << endl;
    out << "tab-id " << this->id << endl;
    out << "parent-tab-id " << this->parent->id << endl;

    for (auto& sym : symbols) {
        out << "var ";
        out << sym->id << " ";
        out << sym->name << " ";
        out << ValueTypeUtils::getName(sym->valueType) << " ";
        out << ValueTypeUtils::getBytes(sym->valueType) << endl;
    }

    out << "% end" << endl;
}

int tcir::BlockSymbolTable::build(
    istream& in, 
    ostream& err,
    map<int, BlockSymbolTable*>& container,
    VariableDescriptionTable* descTable
) {

    std::map<BlockSymbolTable*, int> parentIdMap;

    string token;

    /**
     * 构造一个子表。
     * 子表以 % begin 开头，以 % end 结尾。
     * 其中，第一个 % 会被吞掉。函数读入的第一个字符应该是 begin。
     * 结尾的 end 需要由本函数吞掉。
     * 
     * 函数可以使用“全局”的 token 变量作为缓存。
     */
    auto buildSingle = [&] () {
        in >> token; // 把 begin 干掉。

        auto pSymTab = new BlockSymbolTable;
        pSymTab->descTable = descTable;

        while (true) {
            in >> token;
            if (token == "%") {
                in >> token; // 去掉 end 符号。
                break;
            } else if (token == "tab-id") {
                in >> token;
                pSymTab->id = stoi(token);
            } else if (token == "parent-tab-id") {
                in >> token;
                parentIdMap[pSymTab] = stoi(token);
            } else if (token == "var") {
                string id, name, type, bytes;
                in >> id >> name >> type >> bytes;
                auto sym = new VariableSymbol;
                sym->bytes = stoi(bytes);
                sym->id = stoi(id);
                pSymTab->put(sym);
            } else {
                // todo: 无法识别的符号。
            }
        }

        container[pSymTab->id] = pSymTab;
    };

    while (true) {
        in >> token;
        if (token == "@") {
            in >> token >> token >> token;
            break;
        } else if (token == "%") {
            buildSingle();
        } else {
            err << "[err] bad token: " << token << endl;
            return -3;
        }
    }

    // 母子关系绑定。
    for (auto& it : container) {
        auto& pParent = container[parentIdMap[it.second]];

        if (pParent != it.second) {

            pParent->children.push_back(it.second);
            it.second->parent = pParent;
        }
    }

    return 0;
}
