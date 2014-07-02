#!/bin/bash

#example VERSION=1.2.3.4 name=xxxcoin ./create-deb.sh 
CreateDeb_binariesPath=$PWD/bin/*/
CreateDeb_ROOT=$PWD/deb
CreateDeb_OUT=$PWD
CreateDeb_DEBIAN=$CreateDeb_ROOT/DEBIAN
CreateDeb_CONTROL=$CreateDeb_DEBIAN/control
CreateDeb_DEPS=$(ldd $CreateDeb_binariesPath$name-qt | while read line; do pkg=$( echo "$line" |cut -f 1 -d " "); if [ $pkg != 'linux-vdso.so.1' ]; then echo $(dpkg -S $pkg| head -n1|cut -f 1 -d ":") \(\>= $(dpkg -s $(dpkg -S $pkg| head -n1|cut -f 1 -d ":") |grep 'Version'|cut -f 2 -d " "|cut -f 1 -d "-")\), ; fi; done |sort -u |tr -s '\r\n' ' '|sed 's/..$//') 
 
 
rm -rf $CreateDeb_ROOT
mkdir -p $CreateDeb_DEBIAN || exit 1
 
 
if [ -e $VERSION ]; then
	version=`date +%s`
fi
 
echo "Package: $name
Version: $VERSION
Section: utils
Priority: optional
Architecture: amd64
Depends: $CreateDeb_DEPS
Maintainer: Sergei Lodyagin <serg@kogorta.dp.ua>
Description: ${name^}
 Peer-to-peer cryptocurrency" > $CreateDeb_CONTROL || exit 1
 

mkdir -p $CreateDeb_ROOT/usr/bin || exit 1
cp $CreateDeb_binariesPath* $CreateDeb_ROOT/usr/bin || exit 1

mkdir -p $CreateDeb_ROOT/usr/share/applications/
cat <<EOF >> $CreateDeb_ROOT/usr/share/applications/eclipse.desktop
[Desktop Entry]
Encoding=UTF-8
Name=${name^}
Comment=${name^} P2P Cryptocurrency
Exec=/usr/bin/$name-qt %u
Terminal=false
Type=Application
Icon=/usr/share/pixmaps/$(echo $name)128.png
MimeType=x-scheme-handler/bitcoin;
Categories=Network;Office;
Name[en_US]=$name-qt.desktop
EOF

mkdir -p $CreateDeb_ROOT/usr/share/pixmaps/
cp $CreateDeb_OUT/src/share/pixmaps/$(echo $name)128.png $CreateDeb_ROOT/usr/share/pixmaps/

mkdir -p $CreateDeb_ROOT/usr/share/man/man1/
cp $CreateDeb_OUT/src/contrib/debian/manpages/*.1 $CreateDeb_ROOT/usr/share/man/man1/

mkdir -p $CreateDeb_ROOT/usr/share/man/man5/
cp $CreateDeb_OUT/src/contrib/debian/manpages/*.5 $CreateDeb_ROOT/usr/share/man/man5/


deb_name=$name'-'$VERSION'.deb'
mkdir -p $CreateDeb_OUT  || exit 1 
 
dpkg -b $CreateDeb_ROOT $CreateDeb_OUT/$deb_name
rm -rf $CreateDeb_ROOT
