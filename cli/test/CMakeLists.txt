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
  
	if (MSVC)
        # For MSVC, CMake sets certain flags to defaults we want to override.
        # This replacement code is taken from sample in the CMake Wiki at
        # http://www.cmake.org/Wiki/CMake_FAQ#Dynamic_Replace.
        foreach(flag_var ${compiler_flags})
          if (NOT BUILD_SHARED_LIBS)
            # When DTCenter is built as a shared library, it should also use
            # shared runtime libraries.  Otherwise, it may end up with multiple
            # copies of runtime library data in different modules, resulting in
            # hard-to-find crashes. When it is built as a static library, it is
            # preferable to use CRT as static libraries, as we don't have to rely
            # on CRT DLLs being available. CMake always defaults to using shared
            # CRT libraries, so we override that default here.
            string(REPLACE "/MD" "-MT" ${flag_var} "${${flag_var}}")
          endif ()
          
          # Do not create .pdb files
          string(REPLACE "/Zi" "/Z7" ${flag_var} "${${flag_var}}")
          string(REPLACE "/ZI" "/Z7" ${flag_var} "${${flag_var}}")
          
          # We prefer more strict warning checking for building DTCenter.
          # Replaces /W3 with /W4 in defaults.
          # string(REPLACE "/W3" "/W4" ${flag_var} "${${flag_var}}")
        endforeach()

        # Suppress MSVC warnning
        add_definitions(-wd4996)

        # Summary of tuple support for Microsoft Visual Studio:
        # Compiler    version(MS)  version(cmake)  Support
        # ----------  -----------  --------------  -----------------------------
        # vs 2005      8           1400            disabled warings [4355]
        # <= VS 2010  <= 10        <= 1600         Use Google Tests's own tuple.
        # VS 2012     11           1700            std::tr1::tuple + _VARIADIC_MAX=10
        # VS 2013     12           1800            std::tr1::tuple
        if (MSVC)  
          if (MSVC_VERSION EQUAL 1400)
            add_definitions(-wd4355)
          endif (MSVC_VERSION EQUAL 1400)
          
          if (MSVC_VERSION EQUAL 1700)
            add_definitions(/D _VARIADIC_MAX=10)
          endif (MSVC_VERSION EQUAL 1700)
        endif (MSVC)   

        # Summary of name for Microsoft Visual Studio:
        # Compiler    version(MS)  version(cmake)  Name
        # ----------  -----------  --------------  -----------------------------
        # VS 2005      8           1400            vs2005
        # VS 2008      9           1500            vs2008
        # VS 2010     10           1600            vs2010
        # VS 2012     11           1700            vs2012
        # VS 2013     12           1800            vs2013
        # VS 2015     13           1900            vs2015
        if (MSVC_VERSION EQUAL 1400)
          set(MSVC_NAME "vs2005")
        elseif (MSVC_VERSION EQUAL 1500)
          set(MSVC_NAME "vs2008")
        elseif (MSVC_VERSION EQUAL 1600)
          set(MSVC_NAME "vs2010")
        elseif (MSVC_VERSION EQUAL 1700)
          set(MSVC_NAME "vs2012")
        elseif (MSVC_VERSION EQUAL 1800)
          set(MSVC_NAME "vs2013")
        elseif (MSVC_VERSION EQUAL 1900)
          set(MSVC_NAME "vs2015")
        elseif (MSVC_VERSION EQUAL 1910)
          set(MSVC_NAME "vs2017")
        else ()
          message(FATAL_ERROR "Please add the MSVC_NAME for MSVC_VERSION[${MSVC_VERSION}]")
        endif (MSVC_VERSION EQUAL 1400)

        if (NOT BUILD_SHARED_LIBS)
          set(LIBRARY_POSTFIX "mtd")
        else ()
          set(LIBRARY_POSTFIX "mdd")
        endif (NOT BUILD_SHARED_LIBS)
    endif (MSVC)

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
        add_definitions(-Wall -Wextra -D_GLIBCXX_USE_CXX11_ABI=0)
        add_definitions(-Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-parentheses -Wno-write-strings -Wno-format-security)
        add_definitions(-Wno-sign-compare -Wno-nonnull-compare -Wno-return-type -Wno-comment -Wno-ignored-qualifiers -Wno-missing-field-initializers)
		    #add_definitions(-fprofile-arcs -ftest-coverage)
        # 以下只对C++语言有有效
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -Wno-reorder")

        # GCC 4.3.4不支持 -Wno-conversion-null， 先注释掉
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-conversion-null")

        #add_definitions(-rdynamic)
        
        
  endif(UNIX)
endmacro()
dtcenter_init_complier_settings()
add_definitions(-g -O0)
add_definitions(-DDT_COMPILE_GCC -DAUTOSTAR_LINUX -D_GLIBCXX_USE_CXX11_ABI=0)
add_definitions(-Dgtest)

# GCC编译器告警开关
add_definitions(-Wall -Wextra)
add_definitions(-Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-parentheses -Wno-write-strings -Wno-format-security)
add_definitions(-Wno-sign-compare -Wno-return-type -Wno-comment -Wno-ignored-qualifiers)


set(UT_DEMO_SRCS testcase/main.cpp ../src/main.c testcase/gtest_mytestcase.cpp)

add_executable(ut_demo ${UT_DEMO_SRCS})
target_link_libraries(ut_demo 
	${PROJECT_SOURCE_DIR}/lib/libdtcenter.a
)
target_link_libraries(ut_demo -pthread)
target_include_directories(ut_demo PUBLIC ${PROJECT_SOURCE_DIR}/include)
