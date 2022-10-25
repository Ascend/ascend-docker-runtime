#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
# Description: ascend-docker-runtime构建脚本
set -ex

ROOT=$(cd $(dirname $0); pwd)/..
TOP_DIR=$ROOT/..

OPENSRC=${ROOT}/opensource
PLATFORM=${ROOT}/platform
OUTPUT=${ROOT}/output
BUILD=${ROOT}/build

CLIDIR=${ROOT}/cli
DESTROYDIR=${ROOT}/destroy
CLISRCNAME="main.c"

INSTALLHELPERDIR=${ROOT}/install
INSTALLHELPERSRCNAME="main.go"

HOOKDIR=${ROOT}/hook
HOOKSRCNAME="main.go"

RUNTIMEDIR=${ROOT}/runtime
RUNTIMESRCNAME="main.go"

CLISRCPATH=$(find ${CLIDIR} -name "${CLISRCNAME}")
CLISRCDIR=${CLISRCPATH%/${CLISRCNAME}}
DESTROYSRCPATH=$(find ${DESTROYDIR} -name "${CLISRCNAME}")
DESTROYDIR=${DESTROYSRCPATH%/${CLISRCNAME}}
INSTALLHELPERSRCPATH=$(find ${INSTALLHELPERDIR} -name "${INSTALLHELPERSRCNAME}")
INSTALLHELPERSRCDIR=${INSTALLHELPERSRCPATH%/${INSTALLHELPERSRCNAME}}
HOOKSRCPATH=$(find ${HOOKDIR} -name "${HOOKSRCNAME}")
HOOKSRCDIR=${HOOKSRCPATH%/${HOOKSRCNAME}}
RUNTIMESRCPATH=$(find ${RUNTIMEDIR} -name "${RUNTIMESRCNAME}")
RUNTIMESRCDIR=${RUNTIMESRCPATH%/${RUNTIMESRCNAME}}

VERSION=$(cat $TOP_DIR/Toolbox_CI/config/version.ini | grep "PackageName" | cut -d "=" -f 2)
PACKAGENAME="Ascend-docker-runtime"
CPUARCH=$(uname -m)

function build_bin()
{
    echo "make destroy"
    [ -d "${BUILD}/build/destroy/build" ] && rm -rf ${BUILD}/build/destroy/build
    mkdir -p ${BUILD}/build/destroy/build && cd ${BUILD}/build/destroy/build

    cmake ${DESTROYDIR}
    make clean
    make

    echo "make cli"
    [ -d "${BUILD}/build/cli/build" ] && rm -rf ${BUILD}/build/cli/build
    mkdir -p ${BUILD}/build/cli/build && cd ${BUILD}/build/cli/build

    cmake ${CLISRCDIR}
    make clean
    make

    [ -d "${ROOT}/opensource/src" ] && rm -rf ${ROOT}/opensource/src
    mkdir ${ROOT}/opensource/src
    export GOPATH="${ROOT}/opensource"
    export GO111MODULE=on
    export GONOSUMDB="*"
    export GONOPROXY=*.huawei.com
    export GOFLAGS="-mod=mod"

    echo "make installhelper"
    cd ${INSTALLHELPERSRCDIR}
    [ -d "${BUILD}/build/helper/build" ] && rm -rf ${BUILD}/build/helper/build
    export CGO_ENABLED=1
    export CGO_CFLAGS="-fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -fPIC -ftrapv"
    export CGO_CPPFLAGS="-fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -fPIC -ftrapv"
    export CGO_LDFLAGS="-Wl,-z,now -Wl,-s,--build-id=none -pie"
    mkdir -p ${BUILD}/build/helper/build
    go build -buildmode=pie  -ldflags='-linkmode=external -buildid=IdNetCheck -extldflags "-Wl,-z,now" -w -s' -trimpath -o ${BUILD}/build/helper/build/ascend-docker-plugin-install-helper ${INSTALLHELPERSRCDIR}/${INSTALLHELPERSRCNAME}

    echo "make hook"
    [ -d "${HOOKSRCDIR}/build" ] && rm -rf ${HOOKSRCDIR}/build
    mkdir ${HOOKSRCDIR}/build && cd ${HOOKSRCDIR}/build
    go build -buildmode=pie  -ldflags='-linkmode=external -buildid=IdNetCheck -extldflags "-Wl,-z,now" -w -s' -trimpath  -o ascend-docker-hook ../${HOOKSRCNAME}
    echo `pwd`
    ls

    echo "make runtime"
    cd ${RUNTIMEDIR}
    [ -d "${RUNTIMESRCDIR}/build" ] && rm -rf ${RUNTIMESRCDIR}/build
    mkdir ${RUNTIMESRCDIR}/build&&cd ${RUNTIMESRCDIR}/build
    go build -buildmode=pie  -ldflags='-linkmode=external -buildid=IdNetCheck -extldflags "-Wl,-z,now" -w -s' -trimpath  -o ascend-docker-runtime ../${RUNTIMESRCNAME}
}

function copy_file_output()
{
    cd ${BUILD}
    mkdir -p ${OUTPUT}

    /bin/cp -f {${RUNTIMESRCDIR},${HOOKSRCDIR},${BUILD}/build/helper,${BUILD}/build/cli,${BUILD}/build/destroy}/build/ascend-docker*  ${OUTPUT}
    /bin/cp -f scripts/base.list ${OUTPUT}
    /bin/cp -f scripts/base.list_A500 ${OUTPUT}
    /bin/cp -f scripts/base.list_A200 ${OUTPUT}
    /bin/cp -f scripts/base.list_A200ISoC ${OUTPUT}
    FILECNT=$(ls -l ${OUTPUT} |grep "^-"|wc -l)
    echo "prepare package $FILECNT bins"
    if [ $FILECNT -ne 9 ]; then
        exit 1
    fi
    /bin/cp -rf ${ROOT}/assets ${OUTPUT}
    /bin/cp -f ${ROOT}/README.md ${OUTPUT}
}

function clean()
{
    [ -d "${OUTPUT}" ] && cd ${OUTPUT}&&rm -rf *
}

clean
build_bin
copy_file_output