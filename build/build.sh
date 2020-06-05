#!/bin/bash


ROOT=$(cd `dirname $0`; pwd)/..

OPENSRC=${ROOT}/opensource
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
BINDIR=${DEBPACK}/usr/local/bin
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

funcbuild(){
echo "make cli"
[ -d "${CLISRCDIR}/build" ]&&rm -rf ${CLISRCDIR}/build
mkdir ${CLISRCDIR}/build&&cd ${CLISRCDIR}/build
cmake ../
make clean
make

echo "make runtime"
[ -d "${INSTALLHELPERSRCDIR}/build" ]&&rm -rf ${INSTALLHELPERSRCDIR}/build
mkdir ${INSTALLHELPERSRCDIR}/build&&cd ${INSTALLHELPERSRCDIR}/build
cmake ../
make clean
make

[ -d "${ROOT}/opensource/src" ]&&rm -rf ${ROOT}/opensource/src
mkdir ${ROOT}/opensource/src
/bin/cp -rf ${HOOKSRCDIR}/vendor/* ${ROOT}/opensource/src
export GOPATH="${GOPATH}:${ROOT}/opensource"

[ -d "${HOOKSRCDIR}/build" ]&&rm -rf ${HOOKSRCDIR}/build
mkdir ${HOOKSRCDIR}/build&&cd ${HOOKSRCDIR}/build
go build ../${HOOKSRCNAME}
mv main ascend-docker-hook

[ -d "${RUNTIMESRCDIR}/build" ]&&rm -rf ${RUNTIMESRCDIR}/build
mkdir ${RUNTIMESRCDIR}/build&&cd ${RUNTIMESRCDIR}/build
go build ../${RUNTIMESRCNAME}
mv main ascend-docker-runtime
}

funcmakedeb(){
cd ${BUILD}
mkdir -pv {${DEBDIR},${BINDIR}}
/bin/cp -f  {${RUNTIMESRCDIR},${HOOKSRCDIR},${INSTALLHELPERSRCDIR},${CLISRCDIR}}/build/ascend-docker*  ${BINDIR}
CONPATH=`find ${INSTALLHELPERDIR} -name "control"` 
INSTPATH=`find ${INSTALLHELPERDIR} -name "postinst"` 
/bin/cp -f  ${CONPATH}  ${INSTPATH}  ${DEBDIR}
echo ${INSTPATH}
chmod 555 ${DEBDIR}/postinst
dpkg-deb -b ${DEBPACK} ascenddockertool_1.0.0_i386.deb
DEBS=`find ${BUILD} -name "*.deb"`
/bin/cp ${DEBS} ${OUTPUT} 
} 

funcmakerpm(){
mkdir -pv ${RPMPACK}/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
/bin/cp -f  {${RUNTIMESRCDIR},${HOOKSRCDIR},${INSTALLHELPERSRCDIR},${CLISRCDIR}}/build/ascend-docker* ${RPMSOURCESDIR}
SPECPATH=`find ${INSTALLHELPERDIR} -name "*.spec"` 
dos2unix ${SPECPATH}
/bin/cp -f  ${SPECPATH}  ${RPMSPECDIR}
rpmbuild --define "_topdir ${RPMPACK}"
rpmbuild --showrc | grep topdir 
echo ${RPMPACK}
echo "%_topdir ${RPMPACK}" > ~/.rpmmacros
rpmbuild -bb ${RPMPACK}/SPECS/ascend-docker-plgugin.spec
RPMS=`find ${RPMPACK} -name "*.rpm"`
ARCH=`uname -m`
RPMSNAME=${RPMS##*/}
/bin/cp ${RPMS} ${OUTPUT}/${RPMSNAME}.${ARCH}
 }

funcmakeclean(){
[ -d "${RPMPACK}" ]&&rm -rf ${RPMPACK}
[ -d "${DEBPACK}" ]&&rm -rf ${DEBPACK}
[ -d "${OUTPUT}" ]&&cd ${output}&&rm -rf *
}

funcmakepull(){
cd ${OPENSRC}
wget -O cJSON.tar.gz  https://github.com/DaveGamble/cJSON/archive/v1.7.13.tar.gz --no-check-certificate
}

funcmakeunzip(){
cd ${OPENSRC}
tar -xzvf cJSON*.tar.gz
CJSONS=`find . -name "cJSON.*"`
CJSONSLIB=`find ${INSTALLHELPERDIR} -name cjson -type d` 
/bin/cp -f ${CJSONS} ${CJSONSLIB}
}

funcmakeclean
if [ $1 == "pull" ]; then
funcmakepull
fi
funcmakeunzip
funcbuild
funcmakerpm
#funcmakedeb
