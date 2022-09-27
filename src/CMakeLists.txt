#[[
    ToyCompile 顶级构建文件。
    创建于 2022年9月26日。
]]

cmake_minimum_required(VERSION 3.24.1)
project(ToyCompile)

# 指定使用 C++17。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 子目录。
add_subdirectory(main)
add_subdirectory(core)

# 构建目标。
add_executable(
    ${PROJECT_NAME} main/main.cpp
)

# 包含文件路径。
target_include_directories(
    ${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/lib/include
    ${PROJECT_BINARY_DIR}
)

# 链接库。
target_link_libraries(
    ${PROJECT_NAME} PUBLIC
    core
    main
)
