cmake_minimum_required(VERSION 2.26)
project(ascend-docker-cli C)
set(CMAKE_C_STANDARD 11)
## The common options using by both c and cxx

message(STATUS "CMAKE_SHARED_LIBRARY_LINK_C_FLAGS = " ${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS})
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

include_directories("${PROJECT_SOURCE_DIR}/HuaweiSecureC")
aux_source_directory(. SRC)
add_subdirectory(HuaweiSecureC)
add_executable(ascend-docker-cli ${SRC})
target_compile_options(ascend-docker-cli PRIVATE -fstack-protector-all -fpie)
target_link_libraries(ascend-docker-cli -pie -Wl,-z,now HuaweiSecureC)
