#[[
    core 目录构建文件。
    创建于 2022年9月26日。
]]

file(GLOB_RECURSE core_lib_source_files *.cpp)

add_library(
    core 
    ${core_lib_source_files}
)

target_include_directories(
    core PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_BINARY_DIR}
)

