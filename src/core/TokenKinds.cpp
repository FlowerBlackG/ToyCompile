/*

    创建：2022年10月31日。

*/

#include <tc/core/TokenKinds.h>

using namespace std;

TokenKindUtils& TokenKindUtils::getInstance() {
    static TokenKindUtils instance;
    return instance;
}

TokenKindUtils::TokenKindUtils() {
    this->makeTokenKindMap();
}

void TokenKindUtils::makeTokenKindMap() {

    tokenKindMap.clear();

    /* 
        以下内容部分通过 kotlin 脚本生成。

        ------ begin of kotlin ------

        val lines = inStr.replace(" ", "").split('\n').filter { it.isNotBlank() }
        lines.forEach { line ->
            if (line.startsWith("KEYWORD")) {
                val idx1 = line.indexOf('(') + 1
                val idx2 = line.indexOf(',')
                val subStr = line.substring(idx1, idx2)
                println("tokenKindMap[\"$subStr\"] = TokenKind::kw_$subStr;")
            } else if (line.startsWith("/")) {
                return@forEach
            } else if (line.startsWith("PUNCTUATOR")) {
                //"PUNCTUATOR(greaterequal,        \">=\")\n" +
                val idx1 = line.indexOf('(') + 1
                val idx2 = line.indexOf(',')
                val idx3 = line.indexOf('"') + 1
                val idx4 = line.lastIndexOf('"')

                val sub1 = line.substring(idx1, idx2)
                val sub2 = line.substring(idx3, idx4)
                println("tokenKindMap[\"$sub2\"] = TokenKind::$sub1;")

            }
        }
        
        ------  end of kotlin  ------

        其中，inStr 为 TokenKinds.def 中的部分内容，如：
            PUNCTUATOR(r_square,            "]")
            PUNCTUATOR(l_paren,             "(")
            PUNCTUATOR(r_paren,             ")")
            PUNCTUATOR(l_brace,             "{")

    */
    
    tokenKindMap["["] = TokenKind::l_square;
    tokenKindMap["]"] = TokenKind::r_square;
    tokenKindMap["("] = TokenKind::l_paren;
    tokenKindMap[")"] = TokenKind::r_paren;
    tokenKindMap["{"] = TokenKind::l_brace;
    tokenKindMap["}"] = TokenKind::r_brace;
    tokenKindMap["."] = TokenKind::period;
    tokenKindMap["..."] = TokenKind::ellipsis;
    tokenKindMap["&"] = TokenKind::amp;
    tokenKindMap["&&"] = TokenKind::ampamp;
    tokenKindMap["&="] = TokenKind::ampequal;
    tokenKindMap["*"] = TokenKind::star;
    tokenKindMap["*="] = TokenKind::starequal;
    tokenKindMap["+"] = TokenKind::plus;
    tokenKindMap["++"] = TokenKind::plusplus;
    tokenKindMap["+="] = TokenKind::plusequal;
    tokenKindMap["-"] = TokenKind::minus;
    tokenKindMap["->"] = TokenKind::arrow;
    tokenKindMap["--"] = TokenKind::minusminus;
    tokenKindMap["-="] = TokenKind::minusequal;
    tokenKindMap["~"] = TokenKind::tilde;
    tokenKindMap["!"] = TokenKind::exclaim;
    tokenKindMap["!="] = TokenKind::exclaimequal;
    tokenKindMap["/"] = TokenKind::slash;
    tokenKindMap["/="] = TokenKind::slashequal;
    tokenKindMap["%"] = TokenKind::percent;
    tokenKindMap["%="] = TokenKind::percentequal;
    tokenKindMap["<"] = TokenKind::less;
    tokenKindMap["<<"] = TokenKind::lessless;
    tokenKindMap["<="] = TokenKind::lessequal;
    tokenKindMap["<<="] = TokenKind::lesslessequal;
    tokenKindMap["<=>"] = TokenKind::spaceship;
    tokenKindMap[">"] = TokenKind::greater;
    tokenKindMap[">>"] = TokenKind::greatergreater;
    tokenKindMap[">="] = TokenKind::greaterequal;
    tokenKindMap[">>="] = TokenKind::greatergreaterequal;
    tokenKindMap["^"] = TokenKind::caret;
    tokenKindMap["^="] = TokenKind::caretequal;
    tokenKindMap["|"] = TokenKind::pipe;
    tokenKindMap["||"] = TokenKind::pipepipe;
    tokenKindMap["|="] = TokenKind::pipeequal;
    tokenKindMap["?"] = TokenKind::question;
    tokenKindMap[":"] = TokenKind::colon;
    tokenKindMap[";"] = TokenKind::semi;
    tokenKindMap["="] = TokenKind::equal;
    tokenKindMap["=="] = TokenKind::equalequal;
    tokenKindMap[","] = TokenKind::comma;
    tokenKindMap["#"] = TokenKind::hash;
    tokenKindMap["##"] = TokenKind::hashhash;
    tokenKindMap["#@"] = TokenKind::hashat;
    tokenKindMap[".*"] = TokenKind::periodstar;
    tokenKindMap["->*"] = TokenKind::arrowstar;
    tokenKindMap["::"] = TokenKind::coloncolon;
    tokenKindMap["@"] = TokenKind::at;
    tokenKindMap["<<<"] = TokenKind::lesslessless;
    tokenKindMap[">>>"] = TokenKind::greatergreatergreater;
    tokenKindMap["^^"] = TokenKind::caretcaret;
    tokenKindMap["auto"] = TokenKind::kw_auto;
    tokenKindMap["break"] = TokenKind::kw_break;
    tokenKindMap["case"] = TokenKind::kw_case;
    tokenKindMap["char"] = TokenKind::kw_char;
    tokenKindMap["const"] = TokenKind::kw_const;
    tokenKindMap["continue"] = TokenKind::kw_continue;
    tokenKindMap["default"] = TokenKind::kw_default;
    tokenKindMap["do"] = TokenKind::kw_do;
    tokenKindMap["double"] = TokenKind::kw_double;
    tokenKindMap["else"] = TokenKind::kw_else;
    tokenKindMap["enum"] = TokenKind::kw_enum;
    tokenKindMap["extern"] = TokenKind::kw_extern;
    tokenKindMap["float"] = TokenKind::kw_float;
    tokenKindMap["for"] = TokenKind::kw_for;
    tokenKindMap["goto"] = TokenKind::kw_goto;
    tokenKindMap["if"] = TokenKind::kw_if;
    tokenKindMap["inline"] = TokenKind::kw_inline;
    tokenKindMap["int"] = TokenKind::kw_int;
    tokenKindMap["_ExtInt"] = TokenKind::kw__ExtInt;
    tokenKindMap["_BitInt"] = TokenKind::kw__BitInt;
    tokenKindMap["long"] = TokenKind::kw_long;
    tokenKindMap["register"] = TokenKind::kw_register;
    tokenKindMap["restrict"] = TokenKind::kw_restrict;
    tokenKindMap["return"] = TokenKind::kw_return;
    tokenKindMap["short"] = TokenKind::kw_short;
    tokenKindMap["signed"] = TokenKind::kw_signed;
    tokenKindMap["static"] = TokenKind::kw_static;
    tokenKindMap["struct"] = TokenKind::kw_struct;
    tokenKindMap["switch"] = TokenKind::kw_switch;
    tokenKindMap["typedef"] = TokenKind::kw_typedef;
    tokenKindMap["union"] = TokenKind::kw_union;
    tokenKindMap["unsigned"] = TokenKind::kw_unsigned;
    tokenKindMap["void"] = TokenKind::kw_void;
    tokenKindMap["volatile"] = TokenKind::kw_volatile;
    tokenKindMap["while"] = TokenKind::kw_while;
    tokenKindMap["_Alignas"] = TokenKind::kw__Alignas;
    tokenKindMap["_Alignof"] = TokenKind::kw__Alignof;
    tokenKindMap["_Atomic"] = TokenKind::kw__Atomic;
    tokenKindMap["_Bool"] = TokenKind::kw__Bool;
    tokenKindMap["_Complex"] = TokenKind::kw__Complex;
    tokenKindMap["_Generic"] = TokenKind::kw__Generic;
    tokenKindMap["_Imaginary"] = TokenKind::kw__Imaginary;
    tokenKindMap["_Noreturn"] = TokenKind::kw__Noreturn;
    tokenKindMap["_Static_assert"] = TokenKind::kw__Static_assert;
    tokenKindMap["_Thread_local"] = TokenKind::kw__Thread_local;
    tokenKindMap["__func__"] = TokenKind::kw___func__;
    tokenKindMap["__objc_yes"] = TokenKind::kw___objc_yes;
    tokenKindMap["__objc_no"] = TokenKind::kw___objc_no;
    tokenKindMap["asm"] = TokenKind::kw_asm;
    tokenKindMap["bool"] = TokenKind::kw_bool;
    tokenKindMap["catch"] = TokenKind::kw_catch;
    tokenKindMap["class"] = TokenKind::kw_class;
    tokenKindMap["const_cast"] = TokenKind::kw_const_cast;
    tokenKindMap["delete"] = TokenKind::kw_delete;
    tokenKindMap["dynamic_cast"] = TokenKind::kw_dynamic_cast;
    tokenKindMap["explicit"] = TokenKind::kw_explicit;
    tokenKindMap["export"] = TokenKind::kw_export;
    tokenKindMap["false"] = TokenKind::kw_false;
    tokenKindMap["friend"] = TokenKind::kw_friend;
    tokenKindMap["mutable"] = TokenKind::kw_mutable;
    tokenKindMap["namespace"] = TokenKind::kw_namespace;
    tokenKindMap["new"] = TokenKind::kw_new;
    tokenKindMap["operator"] = TokenKind::kw_operator;
    tokenKindMap["private"] = TokenKind::kw_private;
    tokenKindMap["protected"] = TokenKind::kw_protected;
    tokenKindMap["public"] = TokenKind::kw_public;
    tokenKindMap["reinterpret_cast"] = TokenKind::kw_reinterpret_cast;
    tokenKindMap["static_cast"] = TokenKind::kw_static_cast;
    tokenKindMap["template"] = TokenKind::kw_template;
    tokenKindMap["this"] = TokenKind::kw_this;
    tokenKindMap["throw"] = TokenKind::kw_throw;
    tokenKindMap["true"] = TokenKind::kw_true;
    tokenKindMap["try"] = TokenKind::kw_try;
    tokenKindMap["typename"] = TokenKind::kw_typename;
    tokenKindMap["typeid"] = TokenKind::kw_typeid;
    tokenKindMap["using"] = TokenKind::kw_using;
    tokenKindMap["virtual"] = TokenKind::kw_virtual;
    tokenKindMap["wchar_t"] = TokenKind::kw_wchar_t;

    // 未识别，补充定义。
    tokenKindMap["sizeof"] = TokenKind::kw_sizeof;


    // ToyCompile 特殊定义。
    tokenKindMap["--_identifier_"] = TokenKind::identifier;
    tokenKindMap["--_unknown_"] = TokenKind::unknown;
    tokenKindMap["--_numeric_constant_"] = TokenKind::numeric_constant;
    tokenKindMap["--_char_constant_"] = TokenKind::char_constant;
    tokenKindMap["--_string_literal_"] = TokenKind::string_literal;
    tokenKindMap["--_eof_"] = TokenKind::eof;
    tokenKindMap["--_single_line_comment_"] = TokenKind::single_line_comment;
    tokenKindMap["--_multi_line_comment_"] = TokenKind::multi_line_comment;


}
