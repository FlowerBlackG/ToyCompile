/*
    符号类型

    created on 2022.12.2
*/

#include <core/SymbolKinds.h>

using namespace std;

SymbolKindUtils& SymbolKindUtils::getInstance() {
    static SymbolKindUtils instance;
    return instance;
}

SymbolKindUtils::SymbolKindUtils() {
    this->makeSymbolKindMap();
}

SymbolKindUtils::~SymbolKindUtils() {
    
}

void SymbolKindUtils::makeSymbolKindMap() {
    this->symbolKindMap.clear();

    /*
        以下构造过程使用 Kotlin 脚本生成。

        ------ begin of kotlin ------

        import java.io.File

        fun main(args: Array<String>) {
            val tokenFile = File("grammar tokens.txt")
            val tokenSet = HashSet<String>()
            tokenFile.readLines().filter {
                it.first().isLowerCase()
            }.forEach {
                tokenSet.add(it)
            }

            println("-- def --")
            tokenSet.forEach {
                println("SYM($it)")
            }

            println()
            println("-- mk map --")
            tokenSet.forEach {
                println("    symbolKindMap[\"$it\"] = SymbolKind::$it;")
            }
        }

        ------  end of kotlin  ------

    */

    symbolKindMap["cast_expression"] = SymbolKind::cast_expression;
    symbolKindMap["compound_statement"] = SymbolKind::compound_statement;
    symbolKindMap["type_name"] = SymbolKind::type_name;
    symbolKindMap["init_declarator"] = SymbolKind::init_declarator;
    symbolKindMap["postfix_expression"] = SymbolKind::postfix_expression;
    symbolKindMap["direct_declarator"] = SymbolKind::direct_declarator;
    symbolKindMap["unary_expression"] = SymbolKind::unary_expression;
    symbolKindMap["logical_or_expression"] = SymbolKind::logical_or_expression;
    symbolKindMap["struct_declaration_list"] = SymbolKind::struct_declaration_list;
    symbolKindMap["and_expression"] = SymbolKind::and_expression;
    symbolKindMap["iteration_statement"] = SymbolKind::iteration_statement;
    symbolKindMap["additive_expression"] = SymbolKind::additive_expression;
    symbolKindMap["abstract_declarator"] = SymbolKind::abstract_declarator;
    symbolKindMap["direct_abstract_declarator"] = SymbolKind::direct_abstract_declarator;
    symbolKindMap["assignment_operator"] = SymbolKind::assignment_operator;
    symbolKindMap["multiplicative_expression"] = SymbolKind::multiplicative_expression;
    symbolKindMap["enum_specifier"] = SymbolKind::enum_specifier;
    symbolKindMap["primary_expression"] = SymbolKind::primary_expression;
    symbolKindMap["pointer"] = SymbolKind::pointer;
    symbolKindMap["unary_operator"] = SymbolKind::unary_operator;
    symbolKindMap["declaration_list"] = SymbolKind::declaration_list;
    symbolKindMap["identifier_list"] = SymbolKind::identifier_list;
    symbolKindMap["enumerator_list"] = SymbolKind::enumerator_list;
    symbolKindMap["designator_list"] = SymbolKind::designator_list;
    symbolKindMap["struct_or_union_specifier"] = SymbolKind::struct_or_union_specifier;
    symbolKindMap["block_item"] = SymbolKind::block_item;
    symbolKindMap["exclusive_or_expression"] = SymbolKind::exclusive_or_expression;
    symbolKindMap["declaration_specifiers"] = SymbolKind::declaration_specifiers;
    symbolKindMap["type_qualifier_list"] = SymbolKind::type_qualifier_list;
    symbolKindMap["struct_declarator"] = SymbolKind::struct_declarator;
    symbolKindMap["designation"] = SymbolKind::designation;
    symbolKindMap["struct_or_union"] = SymbolKind::struct_or_union;
    symbolKindMap["equality_expression"] = SymbolKind::equality_expression;
    symbolKindMap["initializer_list"] = SymbolKind::initializer_list;
    symbolKindMap["struct_declarator_list"] = SymbolKind::struct_declarator_list;
    symbolKindMap["parameter_type_list"] = SymbolKind::parameter_type_list;
    symbolKindMap["translation_unit"] = SymbolKind::translation_unit;
    symbolKindMap["init_declarator_list"] = SymbolKind::init_declarator_list;
    symbolKindMap["inclusive_or_expression"] = SymbolKind::inclusive_or_expression;
    symbolKindMap["designator"] = SymbolKind::designator;
    symbolKindMap["conditional_expression"] = SymbolKind::conditional_expression;
    symbolKindMap["selection_statement"] = SymbolKind::selection_statement;
    symbolKindMap["constant_expression"] = SymbolKind::constant_expression;
    symbolKindMap["specifier_qualifier_list"] = SymbolKind::specifier_qualifier_list;
    symbolKindMap["statement"] = SymbolKind::statement;
    symbolKindMap["type_qualifier"] = SymbolKind::type_qualifier;
    symbolKindMap["shift_expression"] = SymbolKind::shift_expression;
    symbolKindMap["enumerator"] = SymbolKind::enumerator;
    symbolKindMap["labeled_statement"] = SymbolKind::labeled_statement;
    symbolKindMap["block_item_list"] = SymbolKind::block_item_list;
    symbolKindMap["external_declaration"] = SymbolKind::external_declaration;
    symbolKindMap["expression"] = SymbolKind::expression;
    symbolKindMap["type_specifier"] = SymbolKind::type_specifier;
    symbolKindMap["expression_statement"] = SymbolKind::expression_statement;
    symbolKindMap["argument_expression_list"] = SymbolKind::argument_expression_list;
    symbolKindMap["parameter_declaration"] = SymbolKind::parameter_declaration;
    symbolKindMap["assignment_expression"] = SymbolKind::assignment_expression;
    symbolKindMap["declaration"] = SymbolKind::declaration;
    symbolKindMap["declarator"] = SymbolKind::declarator;
    symbolKindMap["initializer"] = SymbolKind::initializer;
    symbolKindMap["struct_declaration"] = SymbolKind::struct_declaration;
    symbolKindMap["storage_class_specifier"] = SymbolKind::storage_class_specifier;
    symbolKindMap["logical_and_expression"] = SymbolKind::logical_and_expression;
    symbolKindMap["relational_expression"] = SymbolKind::relational_expression;
    symbolKindMap["parameter_list"] = SymbolKind::parameter_list;
    symbolKindMap["jump_statement"] = SymbolKind::jump_statement;
    symbolKindMap["function_definition"] = SymbolKind::function_definition;

}
