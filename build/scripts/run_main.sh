#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
# Description: ascend-docker-runtime run包脚本文件
set -e

ASCEND_RUNTIME_CONFIG_DIR=/etc/ascend-docker-runtime.d
DOCKER_CONFIG_DIR=/etc/docker
INSTALL_PATH=/usr/local/Ascend/Ascend-Docker-Runtime

function install()
{
    echo 'installing ascend docker runtime'

    if [ ! -d "${INSTALL_PATH}" ]; then
        mkdir -p ${INSTALL_PATH}
    fi

    cp -f ./ascend-docker-runtime ${INSTALL_PATH}/ascend-docker-runtime
    cp -f ./ascend-docker-hook ${INSTALL_PATH}/ascend-docker-hook
    cp -f ./ascend-docker-cli ${INSTALL_PATH}/ascend-docker-cli
    cp -f ./ascend-docker-plugin-install-helper ${INSTALL_PATH}/ascend-docker-plugin-install-helper
    chmod 550 ${INSTALL_PATH}/ascend-docker-runtime
    chmod 550 ${INSTALL_PATH}/ascend-docker-hook
    chmod 550 ${INSTALL_PATH}/ascend-docker-cli
    chmod 550 ${INSTALL_PATH}/ascend-docker-plugin-install-helper

    cp -rf ./assets ${INSTALL_PATH}/assets
    cp -f ./README.md ${INSTALL_PATH}/README.md
    mkdir -p ${INSTALL_PATH}/script
    cp -f ./uninstall.sh ${INSTALL_PATH}/script/uninstall.sh
    chmod 500 ${INSTALL_PATH}/script/uninstall.sh

    if [ -d "${ASCEND_RUNTIME_CONFIG_DIR}" ]; then
        rm -rf ${ASCEND_RUNTIME_CONFIG_DIR}
    fi
    mkdir -p ${ASCEND_RUNTIME_CONFIG_DIR}
    chmod 750 ${ASCEND_RUNTIME_CONFIG_DIR}
    cp -f ./base.list ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
    chmod 440 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list

    echo 'install executable files success'

    if [ ! -d "${DOCKER_CONFIG_DIR}" ]; then
        mkdir -p ${DOCKER_CONFIG_DIR}
    fi

    SRC="${DOCKER_CONFIG_DIR}/daemon.json.${PPID}"
    DST="${DOCKER_CONFIG_DIR}/daemon.json"
    ./ascend-docker-plugin-install-helper add ${DST} ${SRC} ${INSTALL_PATH}/ascend-docker-runtime
    if [ "$?" != "0" ]; then
        echo 'create damon.json failed'
        exit 1
    fi

    mv ${SRC} ${DST} 
    echo 'create damom.json success'
    echo 'please reboot docker daemon to take effect'
    if [ -f "/var/log/ascend_seclog/ascend_toolbox_install.log" ];then
        chmod 600 "/var/log/ascend_seclog/ascend_toolbox_install.log"
    fi
}

function uninstall()
{
    echo 'uninstalling ascend docker runtime'

    if [ ! -d "${INSTALL_PATH}" ]; then
        echo 'WARNING: the specified install path does not exist, skipping'
        exit 0
    fi

    ${INSTALL_PATH}/script/uninstall.sh
    echo 'remove daemon.json setting success'

    [ -n "${INSTALL_PATH}" ] && rm -rf ${INSTALL_PATH}
    echo 'remove executable files success'

    echo 'del damom.json success'
}

function upgrade()
{
    echo 'upgrading ascend docker runtime'

    if [ ! -d "${INSTALL_PATH}" ]; then
        echo 'ERROR: the specified install path does not exist, stopping upgrading'
        exit 1
    fi

    if [ ! -d "${ASCEND_RUNTIME_CONFIG_DIR}" ]; then
        echo 'ERROR: the configuration directory does not exist'
        exit 1
    fi

    cp -f ./ascend-docker-runtime ${INSTALL_PATH}/ascend-docker-runtime
    cp -f ./ascend-docker-hook ${INSTALL_PATH}/ascend-docker-hook
    cp -f ./ascend-docker-cli ${INSTALL_PATH}/ascend-docker-cli
    cp -f ./ascend-docker-plugin-install-helper ${INSTALL_PATH}/ascend-docker-plugin-install-helper
    cp -f ./uninstall.sh ${INSTALL_PATH}/script/uninstall.sh
    cp -f ./base.list ${ASCEND_RUNTIME_CONFIG_DIR}/base.list
    chmod 550 ${INSTALL_PATH}/ascend-docker-runtime
    chmod 550 ${INSTALL_PATH}/ascend-docker-hook
    chmod 550 ${INSTALL_PATH}/ascend-docker-cli
    chmod 550 ${INSTALL_PATH}/ascend-docker-plugin-install-helper
    chmod 500 ${INSTALL_PATH}/script/uninstall.sh
    chmod 440 ${ASCEND_RUNTIME_CONFIG_DIR}/base.list

    echo 'upgrade ascend docker runtime success'
}

INSTALL_FLAG=n
INSTALL_PATH_FLAG=n
UNINSTALL_FLAG=n
UPGRADE_FLAG=n
DEVEL_FLAG=n

while true
do
    case "$3" in
        --install)
            INSTALL_FLAG=y
            shift
            ;;
        --uninstall)
            UNINSTALL_FLAG=y
            shift
            ;;
        --install-path=*)
            INSTALL_PATH_FLAG=y
            INSTALL_PATH=$(echo $3 | cut -d"=" -f2)
            INSTALL_PATH=$(echo ${INSTALL_PATH} | sed "s/\/*$//g")
            shift
            ;;
        --upgrade)
            UPGRADE_FLAG=y
            shift
            ;;
        --devel)
            DEVEL_FLAG=y
            shift
            ;;
        *)
            break
            ;;
    esac
done

# 安装为相对路径时报错
if [ "${INSTALL_PATH}" == ".." ] || [ "${INSTALL_PATH}" == "." ]; then
      echo "error :Please follow the installation address after the --install-path=<Absolute path>"
      exit 1
fi

# 单纯只有--install-path的判定处理
if [ "${INSTALL_PATH_FLAG}" == "y" ] && \
   [ "${INSTALL_FLAG}" == "n" ] && \
   [ "${UNINSTALL_FLAG}" == "n" ] && \
   [ "${UPGRADE_FLAG}" == "n" ] && \
   [ "${DEVEL_FLAG}" == "n" ]; then
      echo "Error:only input <install_path> command. When use --install-path you also need intput --install or --uninstall or --upgrade or --devel"
      exit 1
fi

# 需root用户权限
if [ "${UID}" != "0" ]; then
    echo 'please run with root permission'
    exit 1
fi

if [ "${INSTALL_FLAG}" == "y" ] || [ "${DEVEL_FLAG}" == "y" ]; then
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
