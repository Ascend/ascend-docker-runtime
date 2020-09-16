#!/bin/bash

ROOT=$(cd `dirname $0`; pwd)/..
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

CLISRCPATH=`find ${CLIDIR} -name "${CLISRCNAME}"`
CLISRCDIR=${CLISRCPATH%/${CLISRCNAME}}
INSTALLHELPERSRCPATH=`find ${INSTALLHELPERDIR} -name "${INSTALLHELPERSRCNAME}"`
INSTALLHELPERSRCDIR=${INSTALLHELPERSRCPATH%/${INSTALLHELPERSRCNAME}}
HOOKSRCPATH=`find ${HOOKDIR} -name "${HOOKSRCNAME}"`
HOOKSRCDIR=${HOOKSRCPATH%/${HOOKSRCNAME}}
RUNTIMESRCPATH=`find ${RUNTIMEDIR} -name "${RUNTIMESRCNAME}"`
RUNTIMESRCDIR=${RUNTIMESRCPATH%/${RUNTIMESRCNAME}}

VERSION=`cat $TOP_DIR/CI/config/version.ini | grep "PackageName" | cut -d "=" -f 2`
PACKAGENAME="Ascend-docker-runtime"
CPUARCH=`uname -m`

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
    go build -ldflags "-buildid=IdNetCheck" -trimpath ../${HOOKSRCNAME}
    mv main ascend-docker-hook

    echo "make runtime"
    [ -d "${RUNTIMESRCDIR}/build" ] && rm -rf ${RUNTIMESRCDIR}/build
    mkdir ${RUNTIMESRCDIR}/build&&cd ${RUNTIMESRCDIR}/build
    go build -ldflags "-buildid=IdNetCheck" -trimpath ../${RUNTIMESRCNAME}
    mv main ascend-docker-runtime
}

function build_run_package()
{
    cd ${BUILD}
    mkdir run_pkg

    /bin/cp -f {${RUNTIMESRCDIR},${HOOKSRCDIR},${INSTALLHELPERSRCDIR},${CLISRCDIR}}/build/ascend-docker*  run_pkg
    FILECNT=`ls -l run_pkg |grep "^-"|wc -l`
    echo "prepare package $FILECNT bins"
    if [ $FILECNT -ne 4 ]; then
        exit 1
    fi

    /bin/cp -f scripts/run_main.sh run_pkg
    chmod 550 run_pkg/run_main.sh

    RUN_PKG_NAME="${PACKAGENAME}-${VERSION}-${CPUARCH}.run"
    DATE=`date -u "+%Y-%m-%d"`
    bash makeself.sh --nomd5 --nocrc --help-header scripts/help.info --packaging-date ${DATE} \
    --tar-extra "--mtime=${DATE}" run_pkg "${RUN_PKG_NAME}" ascend-docker-runtime ./run_main.sh
    mv ${RUN_PKG_NAME} ${OUTPUT}
}

function make_clean()
{
    [ -d "${OUTPUT}" ] && cd ${OUTPUT}&&rm -rf *
}

function make_pull()
{
    cd ${OPENSRC}
    wget -O cJSON.tar.gz  https://github.com/DaveGamble/cJSON/archive/v1.7.13.tar.gz --no-check-certificate
}

function make_unzip()
{
    cd ${OPENSRC}
    tar -xzvf cJSON*.tar.gz
    CJSONS=`find . -name "cJSON.*"`
    CJSONSLIB=${INSTALLHELPERDIR}/deb/src/cjson 
    /bin/cp -f ${CJSONS} ${CJSONSLIB}

    unzip makeself-release-*.zip
    rm -f makeself-release-*.zip
    MAKESELF_DIR=`find . -name "makeself-release-*"`
    /bin/cp -f ${MAKESELF_DIR}/makeself.sh ${BUILD}
    /bin/cp -f ${MAKESELF_DIR}/makeself-header.sh ${BUILD}
    cd ${BUILD} && /bin/cp -f scripts/mkselfmodify.patch ./
    patch -p0 < mkselfmodify.patch
    rm -f mkselfmodify.patch

    cd ${PLATFORM}
    tar -xzvf HuaweiSecureC.tar.gz
    SECURECSRC=`find . -name "src"`
    SECURECINC=`find . -name "include"`

    SECURECLIB=${INSTALLHELPERDIR}/deb/src/HuaweiSecureC
    /bin/cp -f ${SECURECSRC}/* ${SECURECLIB}
    /bin/cp -f ${SECURECINC}/* ${SECURECLIB}

    SECURECLIB=${CLIDIR}/src/HuaweiSecureC
    /bin/cp -f ${SECURECSRC}/* ${SECURECLIB}
    /bin/cp -f ${SECURECINC}/* ${SECURECLIB}
}

make_clean
if [ $1 == "pull" ]; then
    make_pull
fi

make_unzip
build_bin
build_run_package
