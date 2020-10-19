cmake_minimum_required(VERSION 2.6)

project(demo)
macro(dtcenter_init_complier_settings)
    set(compiler_flags
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            )


    if(UNIX)
        # 调试开关
        add_definitions(-g)
        # 关闭优化开关
        add_definitions(-O0)
        # 4.8.5编译选项
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

        # ASAN编译选项
        if (BUILD_ASAN)
            add_definitions(-fsanitize=address -fsanitize-recover=all)
            add_definitions(-fno-omit-frame-pointer -fno-stack-protector)
            add_definitions(-fsanitize=leak)
        endif(BUILD_ASAN)

        if (BUILD_FUZZ)
            # 包含FUZZ时，必须使用C++11语法编译
            # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        endif(BUILD_FUZZ)

        # DTCenter定义的宏
        add_definitions(-DDT_COMPILE_GCC -DAUTOSTAR_LINUX)

        # 默认不支持中文用例名，如果要支持请打开以下配置
        # add_definitions(-DDTCENTER_CN2EN)

        # GCC编译器告警开关
        add_definitions(-Wall -Wextra -D_GLIBCXX_USE_CXX11_ABI=1)
        add_definitions(-Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-parentheses -Wno-write-strings -Wno-format-security)
        add_definitions(-Wno-sign-compare -Wno-nonnull-compare -Wno-return-type -Wno-comment -Wno-ignored-qualifiers -Wno-missing-field-initializers)
        add_definitions(-fprofile-arcs -ftest-coverage)
        # 以下只对C++语言有有效
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -Wno-reorder")

        # GCC 4.3.4不支持 -Wno-conversion-null， 先注释掉
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-conversion-null")

        #add_definitions(-rdynamic)


    endif(UNIX)
endmacro()
#dtcenter_init_complier_settings()
#add_definitions(-g -O0)
#add_definitions(-DDT_COMPILE_GCC -DAUTOSTAR_LINUX)
add_definitions(-Dgtest)
#add_definitions(-lgcov --coverage)

# GCC编译器告警开关
#add_definitions(-Wall -Wextra)
#add_definitions(-Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-parentheses -Wno-write-strings -Wno-format-security)
#add_definitions(-Wno-sign-compare -Wno-return-type -Wno-comment -Wno-ignored-qualifiers)
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector -std=c11 -D _GNU_SOURCE -fprofile-arcs")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector -std=c++11 -D _GNU_SOURCE -fprofile-arcs")

add_subdirectory(srclib)
set(UT_DEMO_SRCS testcase/main.cpp testcase/gtest_mytestcase.cpp)

add_executable(ut_demo ${UT_DEMO_SRCS})
target_link_libraries(ut_demo
        ${PROJECT_SOURCE_DIR}/Depend/lib/libgtest.a
        ${PROJECT_SOURCE_DIR}/Depend/lib/libmockcpp.a
        )
target_link_libraries(ut_demo -fprofile-arcs -pthread)
target_include_directories(ut_demo PUBLIC ${PROJECT_SOURCE_DIR}/Depend/googletest/googletest/include)
target_include_directories(ut_demo PUBLIC ${PROJECT_SOURCE_DIR}/Depend/mockcpp/include)
target_include_directories(ut_demo PUBLIC ${PROJECT_SOURCE_DIR}/Depend/HuaweiSecureC/include)
target_link_libraries(ut_demo demolib)
