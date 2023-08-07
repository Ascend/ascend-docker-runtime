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

set -e
LOG_FILE="/var/log/ascend-docker-runtime/installer.log"
echo "[INFO]: Ascend-Docker-Runtime" $(date +%Y%m%d-%H:%M:%S) "start uninstall" | tee >> ${LOG_FILE}
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
    exit 0
fi

${ROOT}/ascend-docker-plugin-install-helper rm ${DST} ${SRC} ${RESERVEDEFAULT} >> ${LOG_FILE} 2>&1
if [ "$?" != "0" ]; then
    echo "[ERROR]: Ascend-Docker-Runtime" $(date +%Y%m%d-%H:%M:%S) " del damon.json failed, please check ${LOG_FILE} for detail"
    exit 1
fi

mv ${SRC} ${DST}

[ -n "${ASCEND_RUNTIME_CONFIG_DIR}" ] && rm -rf ${ASCEND_RUNTIME_CONFIG_DIR}
INSTALL_ROOT_PATH=$(dirname $(dirname ${ROOT}))

if test -d ${INSTALL_ROOT_PATH}
then
    rm -rf ${INSTALL_ROOT_PATH}
    echo "[INFO]: Ascend-Docker-Runtime $(date +%Y%m%d-%H:%M:%S) delete ${INSTALL_ROOT_PATH} succesfull"
fi
echo "[INFO]: Ascend-Docker-Runtime" $(date +%Y%m%d-%H:%M:%S) "uninstall successfully"
