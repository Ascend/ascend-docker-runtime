#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
# Description: ascend-docker-runtime run package script
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


args=($@)
start_arg="${args[0]}"
start_script=${start_arg#*--}

ASCEND_RUNTIME_CONFIG_DIR=/etc/ascend-docker-runtime.d
DOCKER_CONFIG_DIR=/etc/docker
INSTALL_PATH=/usr/local/Ascend/Ascend-Docker-Runtime
readonly INSTALL_LOG_DIR=/var/log/ascend-docker-runtime
readonly INSTALL_LOG_PATH=${INSTALL_LOG_DIR}/installer.log
readonly INSTALL_LOG_PATH_BAK=${INSTALL_LOG_DIR}/installer_bak.log
readonly LOG_SIZE_THRESHOLD=$((20*1024*1024))
readonly PACKAGE_VERSION=REPLACE_VERSION

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

function print_version {
    echo "Ascend-docker-runtime version: ${PACKAGE_VERSION}"
}

function print_help {
    echo "Error input
Usage: ./Ascend-docker-runtime_${PACKAGE_VERSION}_linux-$(uname -m).run [options]
Options:
  --help | -h                   Print this message
  --check|--info|--list|--quiet|--tar|
  --nox11|--noexec|--extract    These parameters are meaningless for Ascend-docker-runtime and
                                will be discarded in the future
  --install                     Install into this system
  --install-path                Specify the installation path (default: /usr/local/Ascend/Ascend-Docker-Runtime),
                                which must be absolute path
  --uninstall                   Uninstall the installed ascend-docker-runtime tool
  --upgrade                     Upgrade the installed ascend-docker-runtime tool
  --install-type=<type>         Only A500, A500A2, A200ISoC, A200IA2 and A200 need to specify
                                the installation type of Ascend-docker-runtime
                                (eg: --install-type=A200IA2, when your product is A200I A2 or A200I DK A2)
  --ce=<ce>                     Only iSula need to specify the container engine(eg: --ce=isula)
                                MUST use with --install or --uninstall
  --version                     Query Ascend-docker-runtime version
"
}

function check_platform {
  plat="$(uname -m)"
  if [[ $start_script =~ $plat ]]; then
    echo "[INFO]: platform($plat) matched!"
    return 0
  else
    echo "[ERROR]: platform($plat) mismatch for $start_script, please check it"
    return 1
  fi
}

function save_install_args() {
    if [ -f "${INSTALL_PATH}"/ascend_docker_runtime_install.info ]; then
        rm "${INSTALL_PATH}"/ascend_docker_runtime_install.info
    fi
    {
      echo -e "version=v${PACKAGE_VERSION}"
      echo -e "arch=$(uname -m)"
      echo -e "os=linux"
      echo -e "path=${INSTALL_PATH}"
      echo -e "build=Ascend-docker-runtime_${PACKAGE_VERSION}-$(uname -m)"
      echo -e "a500=${a500}"
      echo -e "a500a2=${a500a2}"
      echo -e "a200=${a200}"
      echo -e "a200isoc=${a200isoc}"
      echo -e "a200ia2=${a200ia2}"
    } >> "${INSTALL_PATH}"/ascend_docker_runtime_install.info
    chmod 640 ${INSTALL_PATH}/ascend_docker_runtime_install.info
}

function add_so() {
    if grep -qi "ubuntu" "/etc/os-release"; then
      echo "[info]: os is Ubuntu"
      echo -e "\n/usr/lib/aarch64-linux-gnu/libcrypto.so.1.1" >> ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
      echo "/usr/lib/aarch64-linux-gnu/libyaml-0.so.2" >> ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
    elif grep -qi "euler" "/etc/os-release"; then
      echo "[info]: os is Euler/OpenEuler"
      echo -e "\n/usr/lib64/libcrypto.so.1.1" >> ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
      echo "/usr/lib64/libyaml-0.so.2" >> ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
    else
      echo "[ERROR]: not support this os"
      return 1
    fi
}

function install()
{
    echo "[INFO]: installing ascend docker runtime"
    check_platform
    if [[ $? != 0 ]]; then
        log "[ERROR]" "install failed, run package and os not matched in arch"
        exit 1
    fi

    if [ ! -d "${INSTALL_PATH}" ]; then
        mkdir -p -m 750 ${INSTALL_PATH}
    fi
    if [ -L "${INSTALL_PATH}" ]; then
        log "[ERROR]" "install failed, ${INSTALL_PATH} is symbolic link."
        exit 1
    fi
    cp -f ./ascend-docker-runtime ${INSTALL_PATH}/ascend-docker-runtime
    cp -f ./ascend-docker-hook ${INSTALL_PATH}/ascend-docker-hook
    cp -f ./ascend-docker-cli ${INSTALL_PATH}/ascend-docker-cli
    cp -f ./ascend-docker-plugin-install-helper ${INSTALL_PATH}/ascend-docker-plugin-install-helper
    cp -f ./ascend-docker-destroy ${INSTALL_PATH}/ascend-docker-destroy
    chmod 550 ${INSTALL_PATH}/ascend-docker-runtime
    chmod 550 ${INSTALL_PATH}/ascend-docker-hook
    chmod 550 ${INSTALL_PATH}/ascend-docker-cli
    chmod 550 ${INSTALL_PATH}/ascend-docker-plugin-install-helper
    chmod 550 ${INSTALL_PATH}/ascend-docker-destroy

    if [ -L "${INSTALL_PATH}/script" ]; then
        log "[ERROR]" "install failed, ${INSTALL_PATH}/script is symbolic link."
        exit 1
    fi
    mkdir -p -m 750 ${INSTALL_PATH}/assets
    cp -f ./assets/* ${INSTALL_PATH}/assets
    chmod 640 ${INSTALL_PATH}/assets/20230118566.png ${INSTALL_PATH}/assets/20210329102949456.png
    cp -f ./README.md ${INSTALL_PATH}/README.md
    chmod 640 ${INSTALL_PATH}/README.md
    mkdir -p -m 550 ${INSTALL_PATH}/script
    cp -f ./uninstall.sh ${INSTALL_PATH}/script/uninstall.sh
    chmod 500 ${INSTALL_PATH}/script/uninstall.sh

    if [ -d "${ASCEND_RUNTIME_CONFIG_DIR}" ]; then
        rm -rf ${ASCEND_RUNTIME_CONFIG_DIR}
    fi
    if [ -L "${ASCEND_RUNTIME_CONFIG_DIR}" ]; then
        log "[ERROR]" "install failed, ${ASCEND_RUNTIME_CONFIG_DIR} is symbolic link."
        exit 1
    fi
    mkdir -p -m 750 ${ASCEND_RUNTIME_CONFIG_DIR}
    if [ "${a500}" == "y" ]; then
        cp -f ./base.list_A500 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
    elif [ "${a200}" == "y" ]; then
        cp -f ./base.list_A200 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
    elif [ "${a200isoc}" == "y" ]; then
        cp -f ./base.list_A200ISoC ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
    elif [ "${a500a2}" == "y" ]; then
        cp -f ./base.list_A500A2 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
        add_so
        if [[ $? != 0 ]]; then
            log "[ERROR]" "install failed, a500a2 not support this os"
            exit 1
        fi
    elif [ "${a200ia2}" == "y" ]; then
        cp -f ./base.list_A200IA2 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
        add_so
        if [[ $? != 0 ]]; then
            log "[ERROR]" "install failed, a200ia2 not support this os"
            exit 1
        fi
    else
        cp -f ./base.list ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
    fi
    chmod 440 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list

    echo "[INFO]: install executable files success"

    if [ ! -d "${DOCKER_CONFIG_DIR}" ]; then
        mkdir -p -m 750 ${DOCKER_CONFIG_DIR}
    fi

    SRC="${DOCKER_CONFIG_DIR}/daemon.json.${PPID}"
    DST="${DOCKER_CONFIG_DIR}/daemon.json"
    # exit when return code is not 0, if use 'set -e'
    ./ascend-docker-plugin-install-helper add ${DST} ${SRC} ${INSTALL_PATH}/ascend-docker-runtime ${RESERVEDEFAULT} > /dev/null
    if [[ $? != 0 ]]; then
        log "[ERROR]" "install failed, './ascend-docker-plugin-install-helper add ${DST} ${SRC} ${INSTALL_PATH}/ascend-docker-runtime ${RESERVEDEFAULT}' return non-zero"
        exit 1
    fi

    mv -f ${SRC} ${DST}
    log "[INFO]" "${DST} modify success"
    chmod 600 ${DST}
    save_install_args
    echo "[INFO]: Ascend Docker Runtime has been installed in: ${INSTALL_PATH}"
    echo "[INFO]: The version of Ascend Docker Runtime is: ${PACKAGE_VERSION}"
    echo '[INFO]: please reboot daemon and container engine to take effect'
    log "[INFO]" "Ascend Docker Runtime install success"
}

function uninstall()
{
    echo "[INFO]: Uninstalling ascend docker runtime ${PACKAGE_VERSION}"

    if [ ! -d "${INSTALL_PATH}" ]; then
        log "[WARNING]" "uninstall skipping, the specified install path does not exist"
        exit 0
    fi

    ${INSTALL_PATH}/script/uninstall.sh ${ISULA}
    if [[ $? != 0 ]]; then
        log "[ERROR]" "uninstall failed, '${INSTALL_PATH}/script/uninstall.sh ${ISULA}' return non-zero"
        exit 1
    fi

    [ -n "${INSTALL_PATH}" ] && rm -rf ${INSTALL_PATH}
    log "[INFO]" "Ascend Docker Runtime uninstall success"
}

function upgrade()
{
    echo "[INFO]: upgrading ascend docker runtime"
    check_platform
    if [[ $? != 0 ]]; then
        log "[ERROR]" "install failed, run package and os not matched in arch"
        exit 1
    fi

    if [ ! -d "${INSTALL_PATH}" ]; then
        log "[ERROR]" "upgrade failed, the specified install path does not exist, stopping upgrading"
        exit 1
    fi

    if [ ! -d "${ASCEND_RUNTIME_CONFIG_DIR}" ]; then
        log "[ERROR]" "upgrade failed, the configuration directory does not exist"
        exit 1
    fi
    if [ -L "${INSTALL_PATH}" ]; then
        log "[ERROR]" "upgrade failed, ${INSTALL_PATH} is symbolic link."
        exit 1
    fi
    cp -f ./ascend-docker-runtime ${INSTALL_PATH}/ascend-docker-runtime
    cp -f ./ascend-docker-hook ${INSTALL_PATH}/ascend-docker-hook
    cp -f ./ascend-docker-cli ${INSTALL_PATH}/ascend-docker-cli
    cp -f ./ascend-docker-plugin-install-helper ${INSTALL_PATH}/ascend-docker-plugin-install-helper
    cp -f ./ascend-docker-destroy ${INSTALL_PATH}/ascend-docker-destroy
    cp -f ./uninstall.sh ${INSTALL_PATH}/script/uninstall.sh
    chmod 550 ${INSTALL_PATH}/ascend-docker-runtime
    chmod 550 ${INSTALL_PATH}/ascend-docker-hook
    chmod 550 ${INSTALL_PATH}/ascend-docker-cli
    chmod 550 ${INSTALL_PATH}/ascend-docker-plugin-install-helper
    chmod 550 ${INSTALL_PATH}/ascend-docker-destroy
    chmod 500 ${INSTALL_PATH}/script/uninstall.sh
    if [ -f "${INSTALL_PATH}"/ascend_docker_runtime_install.info ]; then
        if [ "$(grep "a500=y" "${INSTALL_PATH}"/ascend_docker_runtime_install.info)" == "a500=y" ];then
            a500=y
            cp -f ./base.list_A500 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
        elif [ "$(grep "a500a2=y" "${INSTALL_PATH}"/ascend_docker_runtime_install.info)" == "a500a2=y" ]; then
            a500a2=y
            cp -f ./base.list_A500A2 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
            add_so
            if [[ $? != 0 ]]; then
                log "[ERROR]" "upgrade failed, a500a2 not support this os"
                exit 1
            fi
        elif [ "$(grep "a200=y" "${INSTALL_PATH}"/ascend_docker_runtime_install.info)" == "a200=y" ]; then
            a200=y
            cp -f ./base.list_A200 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
        elif [ "x$(grep "a200isoc=y" "${INSTALL_PATH}"/ascend_docker_runtime_install.info)" == "xa200isoc=y" ]; then
            a200isoc=y
            cp -f ./base.list_A200ISoC ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
        elif [ "x$(grep "a200ia2=y" "${INSTALL_PATH}"/ascend_docker_runtime_install.info)" == "xa200ia2=y" ]; then
            a200ia2=y
            cp -f ./base.list_A200IA2 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
            add_so
            if [[ $? != 0 ]]; then
                log "[ERROR]" "upgrade failed, a200a2 not support this os"
                exit 1
            fi
        else
            cp -f ./base.list ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
        fi
        save_install_args
    fi
    chmod 440 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list

    echo "[INFO]: Ascend Docker Runtime has been installed in: ${INSTALL_PATH}"
    echo '[INFO]: upgrade ascend docker runtime success'
    echo "[INFO]: The version of Ascend Docker Runtime is: v${PACKAGE_VERSION}"
    log "[INFO]" "Ascend Docker Runtime upgrade success"
}

INSTALL_FLAG=n
INSTALL_PATH_FLAG=n
UNINSTALL_FLAG=n
UPGRADE_FLAG=n
a500=n
a200=n
a200isoc=n
a500a2=n
a200ia2=n
ISULA=none
RESERVEDEFAULT=no
need_help=y

check_log

while true
do
    case "$3" in
        --install)
            if [ "${INSTALL_FLAG}" == "y" ]; then
                log "[ERROR]" "install failed, '--install' Repeat parameter!"
                exit 1
            fi
            need_help=n
            INSTALL_FLAG=y
            shift
            ;;
        --uninstall)
            if [ "${UNINSTALL_FLAG}" == "y" ]; then
                log "[ERROR]" "uninstall failed, '--uninstall' Repeat parameter!"
                exit 1
            fi
            need_help=n
            UNINSTALL_FLAG=y
            shift
            ;;
        --install-path=*)
            if [ "${INSTALL_PATH_FLAG}" == "y" ]; then
                log "[ERROR]" "failed, '--install-path' Repeat parameter!"
                exit 1
            fi
            need_help=n
            INSTALL_PATH_FLAG=y
            INSTALL_PATH=$(echo $3 | cut -d"=" -f2)
            INSTALL_PATH=$(echo ${INSTALL_PATH}/Ascend-Docker-Runtime | sed "s/\/*$//g")
            shift
            ;;
        --upgrade)
            if [ "${UPGRADE_FLAG}" == "y" ]; then
                log "[ERROR]" "upgrade failed, '--upgrade' Repeat parameter!"
                exit 1
            fi
            need_help=n
            UPGRADE_FLAG=y
            shift
            ;;
        --ce=*)
            if [ "${ISULA}" == "isula" ]; then
                log "[ERROR]" "failed, '--ce' Repeat parameter!"
                exit 1
            fi
            need_help=n
            if [ "$3" == "--ce=isula" ]; then
                DOCKER_CONFIG_DIR="/etc/isulad"
                ISULA=isula
                RESERVEDEFAULT=yes
            else
                log "[ERROR]" "failed, Please check the parameter of --ce=<ce>"
                exit 1
            fi
            shift
            ;;
        --install-type=*)
            if [ "${a500}" == "y" ] || [ "${a200}" == "y" ] || [ "${a200isoc}" == "y" ] ||
            [ "${a200ia2}" == "y" ] || [ "${a500a2}" == "y" ]; then
                log "[ERROR]" "failed, '--install-type' Repeat parameter!"
                exit 1
            fi
            need_help=n

            if [ "$3" == "--install-type=A500" ]; then
                a500=y
            elif [ "$3" == "--install-type=A200" ]; then
                a200=y
            elif [ "$3" == "--install-type=A200ISoC" ]; then
                a200isoc=y
            elif [ "$3" == "--install-type=A500A2" ]; then
                a500a2=y
            elif [ "$3" == "--install-type=A200IA2" ]; then
                a200ia2=y
            else
                log "[ERROR]" "failed, Please check the parameter of --install-type=<type>"
                exit 1
            fi
            shift
            ;;
        --version)
            need_help=n
            print_version
            exit 0
            shift
            ;;
        *)
            if [ "x$3" != "x" ]; then
                log "[ERROR]" "failed, Unsupported parameters: $3"
                print_help
                exit 1
            fi
            break
            ;;
    esac
done

# install path must be absolute path
if [[ ! "${INSTALL_PATH}" =~ ^/.* ]]; then
      log "[ERROR]" "failed, Please follow the installation address after the --install-path=<Absolute path>"
      exit 1
fi

# it is not allowed to input only install-path
if [ "${INSTALL_PATH_FLAG}" == "y" ] && \
   [ "${INSTALL_FLAG}" == "n" ] && \
   [ "${UNINSTALL_FLAG}" == "n" ] && \
   [ "${UPGRADE_FLAG}" == "n" ]; then
      log "[ERROR]" "failed, only input <install_path> command. When use --install-path you also need intput --install or --uninstall or --upgrade"
      exit 1
fi

# must run with root permission
if [ "${UID}" != "0" ]; then
    log "[ERROR]" "failed, please run with root permission"
    exit 1
fi

if [ "${INSTALL_FLAG}" == "y" ]; then
    install
    exit 0
fi

if [ "${UNINSTALL_FLAG}" == "y" ]; then
    uninstall
    exit 0
fi

if [ "${UPGRADE_FLAG}" == "y" ]; then
    upgrade
    exit 0
fi

if [ "${need_help}" == "y" ]; then
  print_help
  exit 0
fi