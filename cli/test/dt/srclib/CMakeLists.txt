cmake_minimum_required(VERSION 2.6)

project(demolib)

# 代码覆盖率
# coverage option
OPTION (ENABLE_COVERAGE "Use gcov" ON)
MESSAGE(STATUS ENABLE_COVERAGE=${ENABLE_COVERAGE})
IF(ENABLE_COVERAGE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()
#add_definitions(-Dgtest -lgcov)
add_definitions(-Dgtest)
#set(UT_DEMO_SRCS ../../../src/main.c)
aux_source_directory(../../../src UT_DEMO_SRCS)
include_directories("../../../src/HuaweiSecureC/include")

add_library(${PROJECT_NAME} ${UT_DEMO_SRCS})
target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/libHuaweiSecureC.a)
