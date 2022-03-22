#!/bin/bash
# Copyright(C) Huawei Technologies Co.,Ltd. 2020-2021. All rights reserved.
set -e
umask 077
CUR_DIR=$(dirname "$(readlink -f $0)")
TOP_DIR=$(realpath "${CUR_DIR}"/../../..)
RUNTIME_DIR=${TOP_DIR}/CODE/mindxcheckutils
export GOPATH="${TOP_DIR}/CODE/opensource"
export PATH="${GOPATH}/bin/;$PATH"
export GO111MODULE=on
export GONOSUMDB="*"

function execute_test() {
  cd ${RUNTIME_DIR}
  go mod tidy
  go install github.com/axw/gocov/gocov@v1.0.0
  go install github.com/matm/gocov-html@latest
  go install gotest.tools/gotestsum@latest
  if ! (go test  -mod=mod -gcflags=all=-l -v -race -coverprofile cov.out ${RUNTIME_DIR} >./$file_input); then
    echo '****** go test cases error! ******'
    exit 1
  else
    echo ${file_detail_output}
    ${GOPATH}/bin/gocov convert cov.out | ${GOPATH}/bin/gocov-html >${file_detail_output}
    ${GOPATH}/bin/gotestsum --junitfile "${TOP_DIR}"/test/unit-tests.xml "${RUNTIME_DIR}"/...
  fi
}

file_input='testDockerPlugin.txt'
file_detail_output="${TOP_DIR}/test/api.html"

echo "************************************* Start LLT Test *************************************"
mkdir -p "${TOP_DIR}"/test/
cd "${TOP_DIR}"/test/
if [ -f "$file_detail_output" ]; then
  rm -rf $file_detail_output
fi
if [ -f "$file_input" ]; then
  rm -rf $file_input
fi
execute_test

echo "************************************* End   LLT Test *************************************"

exit 0
