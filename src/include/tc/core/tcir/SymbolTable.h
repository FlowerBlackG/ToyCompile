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

    /**
     * 符号可见性。
     */
    enum class SymbolVisibility {

        /**
         * 内部。如：非全局变量，private 和 static 的函数。
         */
        internal, // 就像 static
        global

    };

    enum class SymbolType {

        /** 函数定义。 */
        functionDefine,

        /** 结构体。 */
        structureDefine,

        /** 变量。 */
        variableDefine,

        /** 函数参数。 */
        functionParam
    };

    struct SymbolBase {
        SymbolType symbolType;
        SymbolVisibility visibility;
        std::string name;
    };

    struct FunctionParamSymbol : SymbolBase {
        ValueType valueType;
        bool isPointer = false;

        /**
         * 是否为变长参数标记。即：...
         */
        bool isVaList = false;
    };

    struct FunctionSymbol : SymbolBase {
        
        ValueType returnType = ValueType::type_void;
        bool isImported = false;
        
        std::vector< FunctionParamSymbol > params;

        /**
         * 寻找函数参数。如果找不到，会返回 nullptr。
         * 
         * @param paramName 参数名。
         * @return 指向参数表内某符号的指针。注意，参数表发生变化时，可能导致该指针失效。 
         */
        FunctionParamSymbol* findParamSymbol(const std::string& paramName);


    };

    struct VariableSymbol : SymbolBase {
        int id;
        int bytes;
        ValueType valueType;

        /** 默认值。对全局变量有效。 */
        long long initValue = 0;
    };
    

    /**
     * 
     * 其接管内部函数表和变量表的内存。
     */
    struct GlobalSymbolTable {

        std::map< std::string, FunctionSymbol* > functions;
        std::map< std::string, VariableSymbol* > variables;

        VariableSymbol* getVariable(const std::string& name);

        void clear();

        ~GlobalSymbolTable();
            
    };

    /**
     * 临时符号描述表。
     * 该结构会接管其内符号的内存。
     */
    struct VariableDescriptionTable {

        std::map< int, VariableSymbol* > symbolMap;

        void put(VariableSymbol* symbol);
        VariableSymbol* get(int id);

        void clear();

        ~VariableDescriptionTable();

    };

    /**
     * 块符号表。
     * 该表不负责管理其中登记的符号的内存，
     * 但它会将符号交给全局变量描述表，后者会管理该符号的内存。
     * 
     *   int fun() {  <- 创建一张块符号表1.
     * 
     *       if (true) {  <- 创建一张块符号表2.
     * 
     *           ...
     * 
     *       }  <- 销毁块符号表2.
     * 
     *   }  <- 销毁块符号表1.
     */
    struct BlockSymbolTable {

        int id = 0;

        /**
         * 指向代码模块的全局变量描述表。
         */
        VariableDescriptionTable* descTable = nullptr;

        std::vector< VariableSymbol* > symbols;
        std::map< std::string, VariableSymbol* > symbolNameMap;

        /**
         * 上级代码块的符号表。如果自己是祖先，则将此值设为自己。
         */
        BlockSymbolTable* parent = this;

        VariableSymbol* get(const std::string& name, bool allowFromParents);
        VariableSymbol* get(int id, bool allowFromParents);

        /**
         * 将一个变量符号加入到符号表内。
         * 加入后，该符号的内存将由符号描述表管理。
         */
        void put(VariableSymbol* symbol);

        /**
         * 导出。
         * 
         * @param out 输出流。
         */
        void dump(std::ostream& out);

        ~BlockSymbolTable();
    };

}
