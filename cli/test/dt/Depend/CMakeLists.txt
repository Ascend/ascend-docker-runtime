CMAKE_MINIMUM_REQUIRED(VERSION 2.8.8)

PROJECT(depend_lib)

# BUILD_SHARED_LIBS is a standard CMake variable, but we declare it here to
# make it prominent in the GUI.
#option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)

# Specify where is boost
option(USE_DTCENTER_BOOST "Use ${CMAKE_CURRENT_LIST_DIR}/boost header files or not." ON)
if(USE_DTCENTER_BOOST)
	set(BOOST_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}")
else()
	set(BOOST_INCLUDE_DIRS "/usr/include" CACHE PATH "path to boost include dir")
endif()

# helper functions and macros used by dtcenter.
#include(${CMAKE_CURRENT_LIST_DIR}/../cmake/internal_utils.cmake)
#dtcenter_init_complier_settings()

# Subdirectories
set(mockcpp_dir mockcpp)
set(gtest_dir googletest/googletest/)
set(gtest_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/${gtest_dir})
set(mockcpp_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/${mockcpp_dir})

# 将gtest, mockcpp 打包成 depend_lib
set(gtest_library $<TARGET_OBJECTS:gtest>)
set(mockcpp_library $<TARGET_OBJECTS:mockcpp>)

# global_stubs
ADD_LIBRARY(ut_main STATIC ${CMAKE_CURRENT_LIST_DIR}/public_stub.cpp
		${CMAKE_CURRENT_LIST_DIR}/ut_main.cpp)
TARGET_INCLUDE_DIRECTORIES(ut_main PUBLIC "${gtest_SOURCE_DIR}/include")
TARGET_INCLUDE_DIRECTORIES(ut_main PUBLIC "${mockcpp_SOURCE_DIR}/include")

add_subdirectory(${gtest_dir} gtest.out)
add_subdirectory(${mockcpp_dir} mockcpp.out)
