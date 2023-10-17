#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
# Description: ascend-docker-runtime uninstall script
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


readonly INSTALL_LOG_DIR=/var/log/ascend-docker-runtime
readonly INSTALL_LOG_PATH=${INSTALL_LOG_DIR}/installer.log
readonly INSTALL_LOG_PATH_BAK=${INSTALL_LOG_DIR}/installer_bak.log
readonly LOG_SIZE_THRESHOLD=$((20*1024*1024))

function check_log {
    if [[ ! -d ${INSTALL_LOG_DIR} ]]; then
        mkdir -p -m 750 ${INSTALL_LOG_DIR}
    fi
    
    if [[ ! -f ${INSTALL_LOG_PATH} ]]; then
        touch ${INSTALL_LOG_PATH}
        chmod 640 ${INSTALL_LOG_PATH}
        return
    fi

    local log_size="$(ls -l ${INSTALL_LOG_PATH} | awk '{ print $5 }')"
    if [[ ${log_size} -ge ${LOG_SIZE_THRESHOLD} ]]; then
        mv -f ${INSTALL_LOG_PATH} ${INSTALL_LOG_PATH_BAK}
        chmod 400 ${INSTALL_LOG_PATH_BAK}
        > ${INSTALL_LOG_PATH}
        chmod 640 ${INSTALL_LOG_PATH}
    fi
}

function log {
    local ip="${SSH_CLIENT%% *}"
    if [ "${ip}" = "" ]; then
        ip="localhost"
    fi
    echo "$1 $2"
    echo "$1 [$(date +'%Y/%m/%d %H:%M:%S')] [uid: ${UID}] [${ip}] [Ascend-Docker-Runtime] $2" >> ${INSTALL_LOG_PATH}
}

check_log

ROOT=$(cd $(dirname $0); pwd)/..
RESERVEDEFAULT=no
if [ "$*" == "isula" ] ; then
  DST='/etc/isulad/daemon.json'
  echo "[INFO]: You will recover iSula's daemon"
  RESERVEDEFAULT=yes
else
  DST='/etc/docker/daemon.json'
  echo "[INFO]: You will recover Docker's daemon"
fi

SRC="${DST}.${PPID}"
ASCEND_RUNTIME_CONFIG_DIR=/etc/ascend-docker-runtime.d

if [ ! -f "${DST}" ]; then
    log "[WARNING]" "uninstall skipping, ${DST} does not exist"
    exit 0
fi

# exit when return code is not 0, if use 'set -e'
${ROOT}/ascend-docker-plugin-install-helper rm ${DST} ${SRC} ${RESERVEDEFAULT} > /dev/null
if [[ $? != 0 ]]; then
    log "[ERROR]" "uninstall failed, '${ROOT}/ascend-docker-plugin-install-helper rm ${DST} ${SRC} ${RESERVEDEFAULT}' return non-zero"
    exit 1
fi

mv -f ${SRC} ${DST}
log "[INFO]" "${DST} modify success"

[ -n "${ASCEND_RUNTIME_CONFIG_DIR}" ] && rm -rf ${ASCEND_RUNTIME_CONFIG_DIR}
INSTALL_ROOT_PATH=$(dirname $(dirname ${ROOT}))

if test -d ${INSTALL_ROOT_PATH}
then
    rm -rf ${INSTALL_ROOT_PATH}
    echo "[INFO]: delete ${INSTALL_ROOT_PATH} successful"
fi
log "[INFO]" "uninstall.sh exec success"
