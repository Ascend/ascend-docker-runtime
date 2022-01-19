#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
# Description: ascend-docker-runtime 卸载脚本
set -e
LOG_FILE="/var/log/ascend_seclog/ascend_toolbox_install.log"
echo "Ascend-Docker-Runtime" $(date +%Y%m%d-%H:%M:%S) "start uninstall"
echo "Ascend-Docker-Runtime" $(date +%Y%m%d-%H:%M:%S) "start uninstall"  >>${LOG_FILE}
ROOT=$(cd $(dirname $0); pwd)/..
DST='/etc/docker/daemon.json'
SRC="${DST}.${PPID}"
ASCEND_RUNTIME_CONFIG_DIR=/etc/ascend-docker-runtime.d

if [ ! -f "${DST}" ]; then
    exit 0
fi

if [ "$?" != "0" ]; then
    echo "Ascend-Docker-Runtime" $(date +%Y%m%d-%H:%M:%S) "ERROR: del damon.json failed"
    echo "Ascend-Docker-Runtime" $(date +%Y%m%d-%H:%M:%S) "ERROR: del damon.json failed"  >>${LOG_FILE}
    exit 1
fi

mv ${SRC} ${DST}

[ -n "${ASCEND_RUNTIME_CONFIG_DIR}" ] && rm -rf ${ASCEND_RUNTIME_CONFIG_DIR}
INSTALL_ROOT_PATH=$(dirname $(dirname ${ROOT}))

if test -d ${INSTALL_ROOT_PATH}
then
    rm -rf ${INSTALL_ROOT_PATH}
    echo "Ascend-Docker-Runtime $(date +%Y%m%d-%H:%M:%S) delete ${INSTALL_ROOT_PATH} succesfull"
    echo "Ascend-Docker-Runtime $(date +%Y%m%d-%H:%M:%S) delete ${INSTALL_ROOT_PATH} succesfull"  >>${LOG_FILE}
fi
echo "Ascend-Docker-Runtime" $(date +%Y%m%d-%H:%M:%S) "uninstall successfully"
echo "Ascend-Docker-Runtime" $(date +%Y%m%d-%H:%M:%S) "uninstall successfully"  >>${LOG_FILE}