execute_process(COMMAND bash ../install)
# 查找当前目录下的所有源文件
# 并将名称保存到 LIB_SRC 变量
aux_source_directory(. LIB_SRC)

#生成链接库
add_library(cjson ${LIB_SRC})

