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

VERSION=$(cat $TOP_DIR/Toolbox_CI/config/version.ini | grep "PackageName" | cut -d "=" -f 2)
PACKAGENAME="Ascend-docker-runtime"
CPUARCH=$(uname -m)

function build_bin()
{
    echo "make cli"
    [ -d "${BUILD}/build/cli/build" ] && rm -rf ${BUILD}/build/cli/build
    mkdir -p ${BUILD}/build/cli/build && cd ${BUILD}/build/cli/build

    cmake ${CLISRCDIR}
    make clean
    make

    echo "make installhelper"
    [ -d "${BUILD}/build/helper/build" ] && rm -rf ${BUILD}/build/helper/build
    mkdir -p ${BUILD}/build/helper/build && cd ${BUILD}/build/helper/build
    cmake ${INSTALLHELPERSRCDIR}
    make clean
    make

    [ -d "${ROOT}/opensource/src" ] && rm -rf ${ROOT}/opensource/src
    mkdir ${ROOT}/opensource/src
    cd ${HOOKDIR}
    export GOPATH="${ROOT}/opensource"
    export GO111MODULE=on
    export GOPROXY="http://mirrors.tools.huawei.com/goproxy/"
    export GONOSUMDB="*"

    echo "make hook"
    go mod tidy
    [ -d "${HOOKSRCDIR}/build" ] && rm -rf ${HOOKSRCDIR}/build
    mkdir ${HOOKSRCDIR}/build && cd ${HOOKSRCDIR}/build
    export CGO_ENABLED=1
    export CGO_CFLAGS="-fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -fPIC -ftrapv"
    export CGO_CPPFLAGS="-fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2 -fPIC -ftrapv"
    export CGO_LDFLAGS="-Wl,-z,now -Wl,-s,--build-id=none -pie"
    go build -buildmode=pie  -ldflags='-linkmode=external -buildid=IdNetCheck -extldflags "-Wl,-z,now" -w -s' -trimpath ../${HOOKSRCNAME}
    mv main ascend-docker-hook
    echo `pwd`
    ls

    echo "make runtime"
    go mod tidy
    cd ${RUNTIMEDIR}
    [ -d "${RUNTIMESRCDIR}/build" ] && rm -rf ${RUNTIMESRCDIR}/build
    mkdir ${RUNTIMESRCDIR}/build&&cd ${RUNTIMESRCDIR}/build
    go build -buildmode=pie  -ldflags='-linkmode=external -buildid=IdNetCheck -extldflags "-Wl,-z,now" -w -s' -trimpath ../${RUNTIMESRCNAME}
    mv main ascend-docker-runtime
}

function build_run_package()
{
    cd ${BUILD}
    mkdir run_pkg

    /bin/cp -f {${RUNTIMESRCDIR},${HOOKSRCDIR},${BUILD}/build/helper,${BUILD}/build/cli}/build/ascend-docker*  run_pkg
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
    bash ${OPENSRC}/makeself-release-2.4.2/makeself.sh --nomd5 --nocrc --help-header scripts/help.info --packaging-date ${DATE} \
    --tar-extra "--mtime=${DATE}" run_pkg "${RUN_PKG_NAME}" ascend-docker-runtime ./run_main.sh
    mv ${RUN_PKG_NAME} ${OUTPUT}
}

function clean()
{
    [ -d "${OUTPUT}" ] && cd ${OUTPUT}&&rm -rf *
}

clean
build_bin
build_run_package
