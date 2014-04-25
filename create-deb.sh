#!/bin/bash
 
CreateDeb_ROOT=$PWD/build/deb
CreateDeb_OUT=$PWD/build/out
CreateDeb_DEBIAN=$CreateDeb_ROOT/DEBIAN
CreateDeb_CONTROL=$CreateDeb_DEBIAN/control
CreateDeb_DEPS=$(ldd $@rhoscoin-qt | while read line; do pkg=$( echo "$line" |cut -f 1 -d " "); if [ $pkg != 'linux-vdso.so.1' ]; then echo $(dpkg -S $pkg| head -n1|cut -f 1 -d ":") \(\>= $(dpkg -s $(dpkg -S $pkg| head -n1|cut -f 1 -d ":") |grep 'Version'|cut -f 2 -d " "|cut -f 1 -d "-")\), ; fi; done |sort -u |tr -s '\r\n' ' '|sed 's/..$//') 
 
 
rm -rf $CreateDeb_ROOT
mkdir -p $CreateDeb_DEBIAN || exit 1
 
 
if [ -e $VERSION ]; then
	version=`date +%s`
fi
 
echo "Package: rhoscoin
Version: $VERSION
Architecture: amd64
Depends: $CreateDeb_DEPS
Maintainer: Sergei Lodyagin <serg@kogorta.dp.ua>
Description: RHOS coin
 Peer-to-peer cryptocurrency" > $CreateDeb_CONTROL || exit 1
 

mkdir -p $CreateDeb_ROOT/usr/bin || exit 1
cp $@* $CreateDeb_ROOT/usr/bin || exit 1
 
deb_name='rhoscoin_'$VERSION'.deb'
 
dpkg -b $CreateDeb_ROOT $CreateDeb_OUT/$deb_name
