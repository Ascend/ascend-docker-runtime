#!/bin/sh
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
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

cd ../Depend/

if [ -d "./lib" ]; then
  rm -rf ./lib
fi

BUILD_DIR="./build"
if [ ! -d "${BUILD_DIR}" ]; then
  mkdir ${BUILD_DIR}
else
  rm -rf ${BUILD_DIR}/*
  echo "prefix clean"
fi
cd ${BUILD_DIR}
cmake ..
make

mkdir ../lib
find . -name "*a"
cp ./lib/libgtest.a ../lib
cp ./mockcpp.out/src/libmockcpp.a ../lib
cp libut_main.a ../lib

cd ..
if [ -d "${BUILD_DIR}" ]; then
    rm -rf ${BUILD_DIR}
fi

cd ../Scripts

