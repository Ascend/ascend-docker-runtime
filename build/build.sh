#!/bin/bash

ROOT=$(cd `dirname $0`; pwd)/..

OPENSRC=${ROOT}/opensource
PLATFORM=${ROOT}/platform
OUTPUT=${ROOT}/output
BUILD=${ROOT}/build

CLIDIR=${ROOT}/cli
CLISRCNAME="main.c"

INSTALLHELPERDIR=${ROOT}/install
INSTALLHELPERSRCNAME="main.c"

HOOKDIR=${ROOT}/hook
HOOKSRCNAME="main.go"

RUNTIMEDIR=${ROOT}/runtime
RUNTIMESRCNAME="main.go"

DEBPACK=${ROOT}/debpack
BINDIR=${DEBPACK}/usr/bin
DEBDIR=${DEBPACK}/DEBIAN

RPMPACK=${ROOT}/rpmpack
RPMSOURCESDIR=${RPMPACK}/SOURCES
RPMSPECDIR=${RPMPACK}/SPECS


CLISRCPATH=`find ${CLIDIR} -name "${CLISRCNAME}"`
CLISRCDIR=${CLISRCPATH%/${CLISRCNAME}}
INSTALLHELPERSRCPATH=`find ${INSTALLHELPERDIR} -name "${INSTALLHELPERSRCNAME}"`
INSTALLHELPERSRCDIR=${INSTALLHELPERSRCPATH%/${INSTALLHELPERSRCNAME}}
HOOKSRCPATH=`find ${HOOKDIR} -name "${HOOKSRCNAME}"`
HOOKSRCDIR=${HOOKSRCPATH%/${HOOKSRCNAME}}
RUNTIMESRCPATH=`find ${RUNTIMEDIR} -name "${RUNTIMESRCNAME}"`
RUNTIMESRCDIR=${RUNTIMESRCPATH%/${RUNTIMESRCNAME}}

VERSION="20.10.0.B011"
RELEASE="1"
PACKAGENAEM="ascend-docker-runtime"
CPUARCH=`uname -m`

funcbuild(){
echo "make cli"
[ -d "${CLISRCDIR}/build" ]&&rm -rf ${CLISRCDIR}/build
mkdir ${CLISRCDIR}/build&&cd ${CLISRCDIR}/build
cmake ../
make clean
make

echo "make installhelper"
[ -d "${INSTALLHELPERSRCDIR}/build" ]&&rm -rf ${INSTALLHELPERSRCDIR}/build
mkdir ${INSTALLHELPERSRCDIR}/build&&cd ${INSTALLHELPERSRCDIR}/build
cmake ../
make clean
make

[ -d "${ROOT}/opensource/src" ]&&rm -rf ${ROOT}/opensource/src
mkdir ${ROOT}/opensource/src
/bin/cp -rf ${HOOKSRCDIR}/vendor/* ${ROOT}/opensource/src
export GOPATH="${GOPATH}:${ROOT}/opensource"
export GO111MODULE=off

echo "make hook"
[ -d "${HOOKSRCDIR}/build" ]&&rm -rf ${HOOKSRCDIR}/build
mkdir ${HOOKSRCDIR}/build&&cd ${HOOKSRCDIR}/build
go build -ldflags "-buildid=IdNetCheck" -trimpath ../${HOOKSRCNAME}
mv main ascend-docker-hook

echo "make runtime"
[ -d "${RUNTIMESRCDIR}/build" ]&&rm -rf ${RUNTIMESRCDIR}/build
mkdir ${RUNTIMESRCDIR}/build&&cd ${RUNTIMESRCDIR}/build
go build -ldflags "-buildid=IdNetCheck" -trimpath ../${RUNTIMESRCNAME}
mv main ascend-docker-runtime
}

fillcontrol(){
sed -i "1i\Package: ${PACKAGENAEM}" ${DEBDIR}/control
sed -i "2a\Architecture: all" ${DEBDIR}/control
sed -i "3a\Version: ${VERSION}" ${DEBDIR}/control
}

funcmakedeb(){
cd ${BUILD}
mkdir -pv {${DEBDIR},${BINDIR}}
/bin/cp -f  {${RUNTIMESRCDIR},${HOOKSRCDIR},${INSTALLHELPERSRCDIR},${CLISRCDIR}}/build/ascend-docker*  ${BINDIR}
FILECNT=`ls -l ${BINDIR} |grep "^-"|wc -l`
echo "prepare package $FILECNT bins"
if [ $FILECNT -ne 4 ]; then
exit 1
fi 
CONPATH=`find ${INSTALLHELPERDIR} -name "control"` 
INSTPATH=`find ${INSTALLHELPERDIR} -name "postinst"` 
RMPATH=`find ${INSTALLHELPERDIR} -name "prerm"`
/bin/cp -f ${CONPATH} ${INSTPATH} ${RMPATH} ${DEBDIR}
echo ${INSTPATH}
fillcontrol
chmod 555 ${DEBDIR}/postinst
chmod 555 ${DEBDIR}/prerm
dpkg-deb -b ${DEBPACK} ${PACKAGENAEM}_${VERSION}-${RELEASE}_${CPUARCH}.deb
DEBS=`find ${BUILD} -name "*.deb"`
/bin/cp ${DEBS} ${OUTPUT} 
} 

funcfillspec(){
sed -i "4a\BuildArch: $CPUARCH" ${RPMSPECDIR}/*.spec
}

funcmakerpm(){
mkdir -pv ${RPMPACK}/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
/bin/cp -f  {${RUNTIMESRCDIR},${HOOKSRCDIR},${INSTALLHELPERSRCDIR},${CLISRCDIR}}/build/ascend-docker* ${RPMSOURCESDIR}
FILECNT=`ls -l ${RPMSOURCESDIR} |grep "^-"|wc -l`
echo "prepare package $FILECNT bins"
if [ $FILECNT -ne 4 ]; then
exit 1
fi 
SPECPATH=`find ${INSTALLHELPERDIR} -name "*.spec"` 
dos2unix ${SPECPATH}
/bin/cp -f  ${SPECPATH}  ${RPMSPECDIR}
funcfillspec
rpmbuild --define "_topdir ${RPMPACK}"
rpmbuild --showrc | grep topdir 
echo ${RPMPACK}
echo "%_topdir ${RPMPACK}" > ~/.rpmmacros
rpmbuild -bb ${RPMPACK}/SPECS/ascend-docker-plgugin.spec
RPMS=`find ${RPMPACK} -name "*.rpm"`
/bin/cp ${RPMS} ${OUTPUT}
 }

funcmakeclean(){
[ -d "${RPMPACK}" ]&&rm -rf ${RPMPACK}
[ -d "${DEBPACK}" ]&&rm -rf ${DEBPACK}
[ -d "${OUTPUT}" ]&&cd ${OUTPUT}&&rm -rf *
}

funcmakepull(){
cd ${OPENSRC}
wget -O cJSON.tar.gz  https://github.com/DaveGamble/cJSON/archive/v1.7.13.tar.gz --no-check-certificate
}

funcmakeunzip(){
cd ${OPENSRC}
tar -xzvf cJSON*.tar.gz
CJSONS=`find . -name "cJSON.*"`
CJSONSLIB=${INSTALLHELPERDIR}/deb/src/cjson 
/bin/cp -f ${CJSONS} ${CJSONSLIB}

cd ${PLATFORM}
tar -xzvf HuaweiSecureC.tar.gz
SECURECSRC=`find . -name "src"`
SECURECINC=`find . -name "include"`

SECURECLIB=${INSTALLHELPERDIR}/deb/src/HuaweiSecureC
/bin/cp -f ${SECURECSRC}/* ${SECURECLIB}
/bin/cp -f ${SECURECINC}/* ${SECURECLIB}

SECURECLIB=${CLIDIR}/src/HuaweiSecureC
/bin/cp -f ${SECURECSRC}/* ${SECURECLIB}
/bin/cp -f ${SECURECINC}/* ${SECURECLIB}
}

funcmakeclean
if [ $1 == "pull" ]; then
funcmakepull
fi

funcmakeunzip
funcbuild

if [ -f /etc/centos-release ]; then
funcmakerpm
fi

if [ -f /etc/debian_version ]; then
funcmakedeb
fi