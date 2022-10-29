#[[
    ToyCompile 顶级构建文件。
    创建于 2022年9月26日。
]]

cmake_minimum_required(VERSION 3.23.2)
project(ToyCompile)


# 指定使用 C++17。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    
    MESSAGE(STATUS "current platform: Linux ")
    # todo

ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    
    MESSAGE(STATUS "current platform: Windows")

    # 关联 boost 库。
    set(BOOST_ROOT "${PROJECT_SOURCE_DIR}/lib/lib/boost-win-x86_64-mingw")
    message(STATUS boost root is: ${BOOST_ROOT})
    link_directories(${BOOST_ROOT})
    
    # 关联 winsock。
    link_libraries(ws2_32 wsock32)

ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")

    MESSAGE(STATUS "current platform: Darwin")
    set(BOOST_ROOT "/usr/local/include/boost") # 设置 boost 库路径。
    # include_directories(/usr/local/include) # 添加头文件搜索路径。
    link_directories(/usr/local/lib) # 添加库文件搜索路径。

ELSE ()

    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")

ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

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
    ${PROJECT_NAME} 
    core
    main
)
