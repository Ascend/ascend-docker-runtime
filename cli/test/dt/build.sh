#!/bin/bash
CUR_DIR=$(cd `dirname $0`;pwd)
SRC_ROOT="../../src"

# 检测执行pre脚本
usage()
{
    echo "usage:"
    echo "./build.sh [module]."
    echo "eg: ./build.sh"
    echo "eg: ./build.sh cli"
    exit 1
}

makepre() 
{
    if [ -f ${CUR_DIR}/../../../googletest.tar.gz ]; then
      cd ${CUR_DIR}/../../../
      tar xf ${CUR_DIR}/../../../googletest.tar.gz
      cd ${CUR_DIR}
    fi
    if [ -d ${CUR_DIR}/../../../googletest ]; then
      cp -rf ${CUR_DIR}/../../../googletest ${CUR_DIR}/Depend/googletest/
    fi

    if [ -f ${CUR_DIR}/../../../mockcpp.tar.gz ]; then
      cd ${CUR_DIR}/../../../
      tar xf ${CUR_DIR}/../../../mockcpp.tar.gz
      cd ${CUR_DIR}
    fi
    if [ -d ${CUR_DIR}/../../../mockcpp ]; then
      cp -rf ${CUR_DIR}/../../../mockcpp/* ${CUR_DIR}/Depend/mockcpp/
    fi

    # 如果没有生成过，则需要生成
    if [ ! -d Depend/lib ]
    then
        echo "-------------make pre begin-------------------"
        cd Scripts
        chmod u+x ./pre.sh
        ./pre.sh
        cd -
        echo "-------------make pre end---------------------"
    fi

    if [ $? -ne 0 ]
    then
        return 1
    else
        return 0
    fi
}

build_huaweisecurec()
{
    echo "-------------build_ut huaweisecurec begin-------------------"
    if [ -f ${CUR_DIR}/../../../HuaweiSecureC.tar.gz ]; then
      cd ${CUR_DIR}/../../../
      tar xf ${CUR_DIR}/../../../HuaweiSecureC.tar.gz
      cd ${CUR_DIR}
    fi
    if [ -d ${CUR_DIR}/../../../HuaweiSecureC/ ]; then
        cp -rf ${CUR_DIR}/../../../HuaweiSecureC/* ${CUR_DIR}/Depend/HuaweiSecureC/
        cp -rf ${CUR_DIR}/../../../HuaweiSecureC/* ${SRC_ROOT}/HuaweiSecureC/
    fi
    cd ${CUR_DIR}/Depend/HuaweiSecureC
    if [ -d ./build ]; then
        rm -rf ./build
    fi
    mkdir ./build
    cd ./build
    cmake ..
    make
    cp libHuaweiSecureC.a ${CUR_DIR}/srclib
    echo "-------------build_ut huaweisecurec end-------------------"
}

build_cli()
{
    echo "-------------build_ut cli begin-------------------"
    cd ${CUR_DIR}
    if [ -d ${CUR_DIR}/build ]; then
        rm -rf ${CUR_DIR}/build
    fi
    mkdir ${CUR_DIR}/build
    cd ${CUR_DIR}/build
    cmake ..
    make
    echo "-------------build_ut cli end---------------------"
}

run_lcov_cli()
{
    echo "-------------run_ut cli begin-------------------"
    cd ${CUR_DIR}/build
    ./ut_demo
    cd ${SRC_ROOT}/..
    lcov --no-external -o result.info -b . -d . -c
    genhtml result.info -o Report
    echo "-------------run_ut cli end-------------------"
}

main()
{
    # 找到SRC目录
    cd ${SRC_ROOT}
    SRC_ROOT=$(pwd)
    cd -
    
    # step_1 输入检测
    if [ $# -gt 1 ]
    then
        usage
        return 
    fi
    # step_2 执行pre脚本
    makepre
    if [ $? -ne 0 ]
    then
        echo "pre failed."
        return 1
    else
        echo "pre succeed."
    fi
    
    echo SRC_ROOT is "$SRC_ROOT"

    # step_3 编译对应模块
    build_huaweisecurec
    build_cli
    if [ $? -ne 0 ]
    then
        echo "build_cli failed."
        return 1
    else
        echo "build_cli succeed."
    fi

    # step_4 执行ut_demo程序和lcov覆盖率
    run_lcov_cli
    if [ $? -ne 0 ]
    then
        echo "run_lcov_cli failed."
        return 1
    else
        echo "run_lcov_cli succeed."
    fi
}

main $*
