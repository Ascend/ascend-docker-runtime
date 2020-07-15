#!/bin/sh
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
cp gtest.out/libgtest.a ../lib
cp mockcpp.out/libmockcpp.a ../lib
cp libut_main.a ../lib

cd ..
if [ -d "${BUILD_DIR}" ]; then
    rm -rf ${BUILD_DIR}
fi

cd ../Scripts

