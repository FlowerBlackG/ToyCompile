#[[
    core 目录构建文件。
    创建于 2022年9月26日。
]]

add_library(
    core 
    Lexer.cpp
    Dfa.cpp
    Token.cpp
)

target_include_directories(
    core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/lib/include
    ${PROJECT_BINARY_DIR}
)

