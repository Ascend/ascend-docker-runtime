#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
# Description: ascend-docker-runtime构建脚本
set -e

ROOT=$(cd $(dirname $0); pwd)/..
TOP_DIR=$ROOT/..

OPENSRC=${ROOT}/opensource
PLATFORM=${ROOT}/platform
OUTPUT=${ROOT}/output
BUILD=${ROOT}/build

CLIDIR=${ROOT}/cli
CLISRCNAME="main.c"

INSTALLHELPERDIR=${ROOT}/install
INSTALLHELPERSRCNAME="main.c"

HOOKDIR=${ROOT}/hook
HOOKSRCNAME="main.go"

RUNTIMEDIR=${ROOT}/runtime
RUNTIMESRCNAME="main.go"

CLISRCPATH=$(find ${CLIDIR} -name "${CLISRCNAME}")
CLISRCDIR=${CLISRCPATH%/${CLISRCNAME}}
INSTALLHELPERSRCPATH=$(find ${INSTALLHELPERDIR} -name "${INSTALLHELPERSRCNAME}")
INSTALLHELPERSRCDIR=${INSTALLHELPERSRCPATH%/${INSTALLHELPERSRCNAME}}
HOOKSRCPATH=$(find ${HOOKDIR} -name "${HOOKSRCNAME}")
HOOKSRCDIR=${HOOKSRCPATH%/${HOOKSRCNAME}}
RUNTIMESRCPATH=$(find ${RUNTIMEDIR} -name "${RUNTIMESRCNAME}")
RUNTIMESRCDIR=${RUNTIMESRCPATH%/${RUNTIMESRCNAME}}

VERSION=$(cat $TOP_DIR/CI/config/version.ini | grep "PackageName" | cut -d "=" -f 2)
PACKAGENAME="Ascend-docker-runtime"
CPUARCH=$(uname -m)

function build_bin()
{
    echo "make cli"
    [ -d "${CLISRCDIR}/build" ] && rm -rf ${CLISRCDIR}/build
    mkdir ${CLISRCDIR}/build && cd ${CLISRCDIR}/build
    cmake ../
    make clean
    make

    echo "make installhelper"
    [ -d "${INSTALLHELPERSRCDIR}/build" ] && rm -rf ${INSTALLHELPERSRCDIR}/build
    mkdir ${INSTALLHELPERSRCDIR}/build && cd ${INSTALLHELPERSRCDIR}/build
    cmake ../
    make clean
    make

    [ -d "${ROOT}/opensource/src" ] && rm -rf ${ROOT}/opensource/src
    mkdir ${ROOT}/opensource/src
    /bin/cp -rf ${HOOKSRCDIR}/vendor/* ${ROOT}/opensource/src
    export GOPATH="${GOPATH}:${ROOT}/opensource"
    export GO111MODULE=off

    echo "make hook"
    [ -d "${HOOKSRCDIR}/build" ] && rm -rf ${HOOKSRCDIR}/build
    mkdir ${HOOKSRCDIR}/build && cd ${HOOKSRCDIR}/build
    export CGO_ENABLED=1
    export CGO_CFLAGS="-fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -fPIC -ftrapv"
    export CGO_CPPFLAGS="-fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -fPIC -ftrapv"
    go build  -ldflags "-buildid=IdNetCheck" -trimpath ../${HOOKSRCNAME}
    strip main
    mv main ascend-docker-hook

    echo "make runtime"
    [ -d "${RUNTIMESRCDIR}/build" ] && rm -rf ${RUNTIMESRCDIR}/build
    mkdir ${RUNTIMESRCDIR}/build&&cd ${RUNTIMESRCDIR}/build
    export CGO_ENABLED=1
    export CGO_CFLAGS="-fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -fPIC -ftrapv"
    export CGO_CPPFLAGS="-fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -fPIC -ftrapv"
    go build   -ldflags "-buildid=IdNetCheck" -trimpath ../${RUNTIMESRCNAME}
    strip main
    mv main ascend-docker-runtime
}

function build_run_package()
{
    cd ${BUILD}
    mkdir run_pkg

    /bin/cp -f {${RUNTIMESRCDIR},${HOOKSRCDIR},${INSTALLHELPERSRCDIR},${CLISRCDIR}}/build/ascend-docker*  run_pkg
    /bin/cp -f scripts/uninstall.sh run_pkg
    /bin/cp -f scripts/base.list run_pkg
    FILECNT=$(ls -l run_pkg |grep "^-"|wc -l)
    echo "prepare package $FILECNT bins"
    if [ $FILECNT -ne 6 ]; then
        exit 1
    fi
    /bin/cp -rf ${ROOT}/assets run_pkg
    /bin/cp -f ${ROOT}/README.md run_pkg
    /bin/cp -f scripts/run_main.sh run_pkg
    chmod 550 run_pkg/run_main.sh

    RUN_PKG_NAME="${PACKAGENAME}-${VERSION}-${CPUARCH}.run"
    DATE=$(date -u "+%Y-%m-%d")
    bash ${OPENSRC}/${MAKESELF_DIR}/makeself.sh --nomd5 --nocrc --help-header scripts/help.info --packaging-date ${DATE} \
    --tar-extra "--mtime=${DATE}" run_pkg "${RUN_PKG_NAME}" ascend-docker-runtime ./run_main.sh
    mv ${RUN_PKG_NAME} ${OUTPUT}
}

function make_clean()
{
    [ -d "${OUTPUT}" ] && cd ${OUTPUT}&&rm -rf *
}

function make_unzip()
{
    cd ${OPENSRC}
    CJSONS=$(find . -name "cJSON.*")
    CJSONSLIB=${INSTALLHELPERDIR}/deb/src/cjson 
    /bin/cp -f ${CJSONS} ${CJSONSLIB}

    MAKESELF_DIR=$(find . -name "makeself-release-*")

    cd ${PLATFORM}/HuaweiSecureC
    SECURECSRC=$(find . -name "src")
    SECURECINC=$(find . -name "include")

    SECURECLIB=${INSTALLHELPERDIR}/deb/src/HuaweiSecureC
    /bin/cp -f ${SECURECSRC}/* ${SECURECLIB}
    /bin/cp -f ${SECURECINC}/* ${SECURECLIB}

    SECURECLIB=${CLIDIR}/src/HuaweiSecureC
    /bin/cp -f ${SECURECSRC}/* ${SECURECLIB}
    /bin/cp -f ${SECURECINC}/* ${SECURECLIB}
}

make_clean
make_unzip
build_bin
build_run_package
