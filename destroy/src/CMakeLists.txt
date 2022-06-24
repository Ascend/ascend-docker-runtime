cmake_minimum_required(VERSION 2.26)
project(ascend-docker-destroy C)
set(CMAKE_C_STANDARD 11)
## The common options using by both c and cxx

add_compile_options(-fstack-protector-all -D _GNU_SOURCE -Wl,--no-undefined)

message(STATUS "CMAKE_SHARED_LIBRARY_LINK_C_FLAGS = " ${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS})
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
include_directories("${PROJECT_SOURCE_DIR}/../../platform/HuaweiSecureC/include")
aux_source_directory(. SRC)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../build/HuaweiSecureC ${CMAKE_CURRENT_SOURCE_DIR}/../../build/HuaweiSecureC)
add_executable(ascend-docker-destroy ../../cli/src/logger.c ../../cli/src/utils.c ${SRC} )
target_compile_options(ascend-docker-destroy PRIVATE -fstack-protector-all -fpie -ldl -D_FORTIFY_SOURCE=2 -O2)
target_link_libraries(ascend-docker-destroy -ldl -pie -Wl,-s,-z,now HuaweiSecureC)
