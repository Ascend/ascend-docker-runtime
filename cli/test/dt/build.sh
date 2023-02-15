#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================

CUR_DIR=$(cd `dirname $0`;pwd)
SRC_ROOT="../../src"

# execute pre script
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
      mv googletest-release-1.10.0/ googletest
      cd ${CUR_DIR}
    fi
    if [ -d ${CUR_DIR}/../../../googletest ]; then
      cp -rf ${CUR_DIR}/../../../googletest/* ${CUR_DIR}/Depend/googletest/
    fi

    if [ -f ${CUR_DIR}/../../../mockcpp.tar.gz ]; then
      cd ${CUR_DIR}/../../../
      tar xf ${CUR_DIR}/../../../mockcpp.tar.gz
      cd ${CUR_DIR}
    fi
    if [ -d ${CUR_DIR}/../../../mockcpp ]; then
      cp -rf ${CUR_DIR}/../../../mockcpp/* ${CUR_DIR}/Depend/mockcpp/
      cd ${CUR_DIR}/Depend/mockcpp
      sed -i '16s/SET(MOCKCPP_SRC_ROOT ${CMAKE_SOURCE_DIR})/SET(MOCKCPP_SRC_ROOT ${CMAKE_SOURCE_DIR}\/mockcpp)/' ./src/CMakeLists.txt
      cmake .
      make
      cd ${CUR_DIR}
    fi

    # need to compile lib if it is not present
    if [ ! -d Depend/lib ]
    then
        echo "-------------make pre begin-------------------"
        cd Scripts
        chmod u+x ./pre.sh
        bash -ex ./pre.sh
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

build_boundscheck()
{
    echo "-------------build_ut boundscheck begin-------------------"
    if [ -f ${CUR_DIR}/../../../libboundscheck.tar.gz ]; then
      cd ${CUR_DIR}/../../../
      tar xf ${CUR_DIR}/../../../libboundscheck.tar.gz
      mv libboundscheck-v1.1.10/ libboundscheck
      cd ${CUR_DIR}
    fi
    if [ -d ${CUR_DIR}/../../../libboundscheck ]; then
        cp -rf ${CUR_DIR}/../../../libboundscheck/* ${CUR_DIR}/Depend/libboundscheck/
        cp -rf ${CUR_DIR}/../../../libboundscheck/* ${SRC_ROOT}/libboundscheck/
    fi
    cd ${CUR_DIR}/Depend/libboundscheck
    if [ -d ./build ]; then
        rm -rf ./build
    fi
    mkdir ./build
    cd ./build
    cmake ..
    make
    cp liblibboundscheck.a ${CUR_DIR}/srclib
    echo "-------------build_ut boundscheck end-------------------"
}

build_cli()
{
    echo "-------------build_ut cli begin-------------------"
    cd ${CUR_DIR}
    if [ -d ${CUR_DIR}/build ]; then
        rm -rf ${CUR_DIR}/build
    fi
    pwd
    find ../.. -name "*a"
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
    ./ut_demo --gtest_output=xml:${CUR_DIR}/test_detail.xml
    cd ${SRC_ROOT}/..
    ENABLE_BRANCH_COV="--rc lcov_branch_coverage=1"
    lcov --no-external -o result.info -b . -d . -c $ENABLE_BRANCH_COV
    genhtml --branch-coverage result.info -o Report $ENABLE_BRANCH_COV
    cd ${CUR_DIR}
    mkdir xml
    cp -f test_detail.xml ./xml/test_detail.xml
    mkdir html
    cp -rf ${SRC_ROOT}/../Report/* ./html/
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
    build_boundscheck
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
