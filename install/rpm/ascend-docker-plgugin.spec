Name: ascenddockerplugin
Version: 1.0.0
Release: 1
BuildArch: noarch
Summary: simple RPM package
License: FIXME

%define _binaries_in_noarch_packages_terminate_build   0

%description
ascend-docker-plugin helps usrs to use NPU in docker

%prep

%build

%install
mkdir -p %{buildroot}/usr/local/bin/
cp -rf %{buildroot}/../../SOURCES/ascend*  %{buildroot}/usr/local/bin/

%pre

%post
#!/bin/bash
DIR=/etc/docker
SRC="${DIR}/daemon.json.${PPID}"
DST="${DIR}/daemon.json"
BINDIR=/usr/local/bin
${BINDIR}/ascend-docker-plugin-install-helper ${DST} ${SRC}
if [ "$?" != "0" ]; then
echo "create damon.json failed\n"
exit 1
fi
\mv ${SRC} ${DST} 
echo "create damom.json success\n"

%preun

%postun

%clean

%files
%defattr(0755,root,root,0755)
/usr/local/bin/*
