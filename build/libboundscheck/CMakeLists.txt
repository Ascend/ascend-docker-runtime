# 查找当前目录下的所有源文件
# 并将名称保存到 LIB_SRC 变量
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../../platform/libboundscheck/src LIB_SRC)

#生成链接库
add_library(libboundscheck ${LIB_SRC})
target_compile_options(libboundscheck PRIVATE -fstack-protector-all -fpie)
