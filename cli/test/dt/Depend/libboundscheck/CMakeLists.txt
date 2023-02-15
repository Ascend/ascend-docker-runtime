cmake_minimum_required(VERSION 3.0)
# 查找当前目录下的所有源文件
# 并将名称保存到 LIB_SRC 变量
aux_source_directory(src LIB_SRC)
include_directories(include)

#生成链接库
add_library(libboundscheck STATIC ${LIB_SRC})
target_compile_options(libboundscheck PRIVATE -fstack-protector-all -fpie)
