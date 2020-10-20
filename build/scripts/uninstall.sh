#!/bin/bash

ROOT=$(cd `dirname $0`; pwd)/..
DST='/etc/docker/daemon.json'
SRC="${DST}.${PPID}"
ASCEND_RUNTIME_CONFIG_DIR=/etc/ascend-docker-runtime.d

if [ ! -f "${DST}" ]; then
    exit 0
fi

${ROOT}/ascend-docker-plugin-install-helper rm ${DST} ${SRC}
if [ "$?" != "0" ]; then
    echo 'ERROR: del damon.json failed'
    exit 1
fi

mv ${SRC} ${DST}

rm -rf ${ASCEND_RUNTIME_CONFIG_DIR}
