# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (ascend-docker-plugin-install-helper)

#导入头文件所在路径
#PROJECT_SOURCE_DIR为cmake宏
include_directories("${PROJECT_SOURCE_DIR}/cjson")

# 查找当前目录下的所有源文件
# 并将名称保存到 SRC 变量
aux_source_directory(. SRC)

# 指定生成目标
add_executable(ascend-docker-plugin-install-helper ${SRC})

# 添加 math 子目录
add_subdirectory(cjson)

# 添加链接库 
#该命令要在add_executable命令下方，否则报错
target_link_libraries(ascend-docker-plugin-install-helper cjson)

