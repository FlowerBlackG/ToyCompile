#[[
    main 目录构建文件。
    创建于 2022年9月26日。

    注意：main.cpp 用顶级构建文件包含。
]]

add_library(
    main 

    # LexerServer
    LexerServer/LexerServer.cpp

    # LexerCli
    LexerCli/LexerCli.cpp
)

target_include_directories(
    main PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/lib/include
)

# 链接库。
target_link_libraries(
    main 
    core
)

