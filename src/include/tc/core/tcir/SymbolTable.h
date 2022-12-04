/*
    符号表

    created on 2022.12.3
*/

#pragma once

#include <vector>
#include <string>
#include <map>

#include <tc/core/tcir/ValueType.h>

namespace tc::tcir {

    enum class SymbolVisibility {

        internal, // 就像 static
        global

    };

    enum class SymbolType {
        functionDefine,
        structureDefine,
        variableDefine
    };

    struct SymbolBase {
        SymbolType symbolType;
        SymbolVisibility visibility;
        std::string name;
    };

    struct FunctionParamSymbol {
        ValueType type;
        bool isPointer = false;

        /**
         * 是否为变长参数标记。即：...
         */
        bool isVaList = false;
        std::string name;
    };

    struct FunctionSymbol : SymbolBase {
        
        ValueType returnType = ValueType::type_void;
        bool isImported = false;
        
        std::vector< FunctionParamSymbol > params;




    };

    struct VariableSymbol : SymbolBase {
        int id;
        int bytes;
        ValueType valueType;

    };
    

    /**
     * 
     * 其接管内部函数表和变量表的内存。
     */
    struct GlobalSymbolTable {

        std::map< std::string, FunctionSymbol* > functions;
        std::map< std::string, VariableSymbol* > variables;

        void clear();

        ~GlobalSymbolTable();
            
    };

    struct VariableDescriptionTable {

        std::map< int, VariableSymbol* > symbolMap;

        void put(VariableSymbol* symbol);
        VariableSymbol* get(int id);

        void clear();

        ~VariableDescriptionTable();

    };


    struct BlockSymbolTable {

        VariableDescriptionTable* descTable = nullptr;

        std::vector< VariableSymbol* > symbols;
        std::map< std::string, VariableSymbol* > symbolNameMap;

        BlockSymbolTable* parent = this;

        VariableSymbol* get(const std::string& name);
        VariableSymbol* get(int id);

        /**
         * 将一个变量符号加入到符号表内。
         * 加入后，该符号的内存将由符号描述表管理。
         */
        void create(VariableSymbol* symbol);

        ~BlockSymbolTable();
    };

}
