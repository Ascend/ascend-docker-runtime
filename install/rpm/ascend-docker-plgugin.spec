Name: ascend-docker-runtime
Version: 1.0.0
Release: 1
Summary: simple RPM package
License: FIXME

%define _binaries_in_noarch_packages_terminate_build   0

%description
ascend-docker-plugin helps usrs to use NPU in docker

%prep

%build

%install
mkdir -p %{buildroot}/usr/bin/
cp -rf %{buildroot}/../../SOURCES/ascend*  %{buildroot}/usr/bin/

%pre

%post
#!/bin/bash
DIR=/etc/docker
SRC="${DIR}/daemon.json.${PPID}"
DST="${DIR}/daemon.json"
BINDIR=/usr/bin
if [ ! -d "${DIR}" ]; then
mkdir ${DIR}
fi
${BINDIR}/ascend-docker-plugin-install-helper add ${DST} ${SRC}
if [ "$?" != "0" ]; then
echo "create damon.json failed\n"
exit 1
fi
\mv ${SRC} ${DST} 
echo "create damom.json success\n"

%preun
#!/bin/bash
DIR=/etc/docker
BINDIR=/usr/bin
SRC="${DIR}/daemon.json.${PPID}"
DST="${DIR}/daemon.json"
${BINDIR}/ascend-docker-plugin-install-helper rm ${DST} ${SRC}
if [ "$?" != "0" ]; then
echo "del damon.json failed\n"
exit 1
fi
\mv ${SRC} ${DST} 
echo "del damom.json success\n"

%postun

%clean

%files
%defattr(0755,root,root,0755)
/usr/bin/*
