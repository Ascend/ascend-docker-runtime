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
readonly ASCEND_RUNTIME_CONFIG_DIR=/etc/ascend-docker-runtime.d

function check_log {
    if [[ ! -d ${INSTALL_LOG_DIR} ]]; then
        mkdir -p -m 750 ${INSTALL_LOG_DIR}
    fi

    check_sub_path ${INSTALL_LOG_DIR}
    if [[ $? != 0 ]]; then
        echo "[ERROR]: ${INSTALL_LOG_DIR} is invalid"
        exit 1
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

function check_path {
    local path="$1"
    if [[ ${#path} -gt 1024 ]] || [[ ${#path} -le 0 ]]; then
        echo "[ERROR]: parameter is invalid, length not in 1~1024"
        return 1
    fi
    if [[ -n $(echo "${path}" | grep -Ev '^[a-zA-Z0-9./_-]*$') ]]; then
        echo "[ERROR]: parameter is invalid, char not all in 'a-zA-Z0-9./_-'"
        return 1
    fi
    path=$(realpath -m -s "${path}")
    while [[ ! -e "${path}" ]]; do
        path=$(dirname "${path}")
    done
    while true; do
        if [[ "${path}" == "/" ]]; then
            break
        fi
        check_path_permission "${path}"
        if [[ $? != 0 ]]; then
            return 1
        fi
        path=$(dirname "${path}")
    done
}

function check_sub_path {
    local path="$1"
    while [[ ! -e "${path}" ]]; do
        return 1
    done
    for file in $(find "${path}"); do
        check_path_permission "${file}"
        if [[ $? != 0 ]]; then
            return 1
        fi
    done
}

function check_path_permission {
    local path="$1"
    if [[ -L "${path}" ]]; then
        echo "[ERROR]: ${path} is soft link"
        return 1
    fi
    if [[ $(stat -c %u "${path}") != 0 ]] || [[ "$(stat -c %g ${path})" != 0 ]]; then
        echo "[ERROR]: user or group of ${path} is not root"
        return 1
    fi
    local permission=$(stat -c %A "${path}")
    if [[ $(echo "${permission}" | cut -c6) == w ]] || [[ $(echo "${permission}" | cut -c9) == w ]]; then
        echo "[ERROR]:  group or other of ${path} has write permisson"
        return 1
    fi
}

check_log

# must run with root permission
if [ "${UID}" != "0" ]; then
    log "[ERROR]" "failed, please run with root permission"
    exit 1
fi

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

if [ ! -f "${DST}" ]; then
    log "[WARNING]" "uninstall skipping, ${DST} does not exist"
    exit 0
fi

check_path ${DST}
if [[ $? != 0 ]]; then
    log "[ERROR]" "uninstall failed, ${DST} is invalid"
    exit 1
fi

# exit when return code is not 0, if use 'set -e'
${ROOT}/ascend-docker-plugin-install-helper rm ${DST} ${SRC} ${RESERVEDEFAULT} > /dev/null
if [[ $? != 0 ]]; then
    log "[ERROR]" "uninstall failed, '${ROOT}/ascend-docker-plugin-install-helper rm ${DST} ${SRC} ${RESERVEDEFAULT}' return non-zero"
    exit 1
fi

mv -f ${SRC} ${DST}
log "[INFO]" "${DST} modify success"

check_path ${ASCEND_RUNTIME_CONFIG_DIR}
if [[ $? != 0 ]]; then
    log "[ERROR]" "uninstall failed, ${ASCEND_RUNTIME_CONFIG_DIR} is invalid"
    exit 1
fi
[ -d "${ASCEND_RUNTIME_CONFIG_DIR}" ] && rm -rf ${ASCEND_RUNTIME_CONFIG_DIR}

INSTALL_ROOT_PATH=$(dirname $(dirname ${ROOT}))
check_path ${INSTALL_ROOT_PATH}
if [[ $? != 0 ]]; then
    log "[ERROR]" "uninstall failed, ${INSTALL_ROOT_PATH} is invalid"
    exit 1
fi

if test -d ${INSTALL_ROOT_PATH}
then
    rm -rf ${INSTALL_ROOT_PATH}
    echo "[INFO]: delete ${INSTALL_ROOT_PATH} successful"
fi
log "[INFO]" "uninstall.sh exec success"
