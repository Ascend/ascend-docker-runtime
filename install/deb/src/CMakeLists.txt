# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (ascend-docker-plugin-install-helper)

message(STATUS "CMAKE_SHARED_LIBRARY_LINK_C_FLAGS = " ${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS})
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")


#导入头文件所在路径
#PROJECT_SOURCE_DIR为cmake宏
include_directories("../../../opensource/cJSON-1.7.14")
include_directories("../../../platform/HuaweiSecureC/include")

# 查找当前目录下的所有源文件
# 并将名称保存到 SRC 变量
aux_source_directory(. SRC)

# 指定生成目标
add_executable(ascend-docker-plugin-install-helper ${SRC})

# 添加子目录
add_subdirectory(cjson)
add_subdirectory(HuaweiSecureC)

# 添加链接库 
#该命令要在add_executable命令下方，否则报错
target_compile_options(ascend-docker-plugin-install-helper PRIVATE -fstack-protector-all -fpie)
target_link_libraries(ascend-docker-plugin-install-helper -pie -Wl,-s,-z,now cjson)
target_link_libraries(ascend-docker-plugin-install-helper -pie -Wl,-s,-z,now HuaweiSecureC)

