#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
# Description: ascend-docker-runtime build script
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

PACKAGENAME="Ascend-docker-runtime"
VERSION="3.0.0"

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
    if [ -d "run_pkg" ]; then
      rm -r run_pkg
    fi
    mkdir run_pkg

    /bin/cp -f {${RUNTIMESRCDIR},${HOOKSRCDIR},${BUILD}/build/helper,${BUILD}/build/cli,${BUILD}/build/destroy}/build/ascend-docker*  run_pkg
    /bin/cp -f scripts/uninstall.sh run_pkg
    /bin/cp -f scripts/base.list run_pkg
    /bin/cp -f scripts/base.list_A500 run_pkg
    /bin/cp -f scripts/base.list_A200 run_pkg
    /bin/cp -f scripts/base.list_A200ISoC run_pkg

    /bin/cp -rf ${ROOT}/assets run_pkg
    /bin/cp -f ${ROOT}/README.md run_pkg
    /bin/cp -f scripts/run_main.sh run_pkg

    chmod 550 run_pkg/run_main.sh

    RUN_PKG_NAME="${PACKAGENAME}_${VERSION}_linux-${CPUARCH}.run"
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
copy_file_output