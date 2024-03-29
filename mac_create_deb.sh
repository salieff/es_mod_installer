#!/bin/sh

set -xe

if [ "$#" -lt '1' ]; then
    echo "Usage: $0 <ipa or deb file>"
    exit 1
fi

rm -rf '/tmp/deb-package'
cp -r 'deb-package' '/tmp'

EXT="${1##*.}"

if [ "$EXT" = "ipa" ]; then
    unzip "$1" -d '/tmp/deb-package'
    rm -rf '/tmp/deb-package/Applications/ESManager.app'
    mv -f '/tmp/deb-package/Payload/ESManager.app' '/tmp/deb-package/Applications/'
    rm -rf '/tmp/deb-package/Payload'
elif [ "$EXT" = "deb" ]; then
    dpkg -x "$1" '/tmp/deb-package'
else
    echo "Error! Only IPA or DEB input archives supported!"
    exit 1
fi

cp 'bundleid/bundleid' '/tmp/deb-package/Applications/ESManager.app/'

find '/tmp/deb-package/Applications' -type f -exec md5sum '{}' ';' | sed -e 's;/tmp/deb-package;;' > '/tmp/deb-package/DEBIAN/md5sums'

SIZEKB=`du -skL '/tmp/deb-package/Applications/ESManager.app' | awk '{ print $1; }'`
perl -p -i -e 's/Installed-Size: \d+/Installed-Size: '"${SIZEKB}"'/g' '/tmp/deb-package/DEBIAN/control'

VERS_MAJ=`cat 'version.h' | grep 'ESM_VERSION_MAJOR' | sed -e 's/#define ESM_VERSION_MAJOR //'`
VERS_MIN=`cat 'version.h' | grep 'ESM_VERSION_MINOR' | sed -e 's/#define ESM_VERSION_MINOR //'`
VERS_BLD=`cat 'version.h' | grep 'ESM_VERSION_BUILD' | sed -e 's/#define ESM_VERSION_BUILD //'`
VERS="${VERS_MAJ}.${VERS_MIN}-${VERS_BLD}"
perl -p -i -e 's/Version: [\d\.-]+/Version: '"${VERS}"'/g' '/tmp/deb-package/DEBIAN/control'

NAME=`cat './deb-package/DEBIAN/control' | grep 'Package:' | sed -e 's/Package: //'`

rm -f "/tmp/${NAME}_${VERS}_iphoneos-arm.deb"
fakeroot dpkg-deb -Zgzip --build '/tmp/deb-package' "/tmp/${NAME}_${VERS}_iphoneos-arm.deb"
