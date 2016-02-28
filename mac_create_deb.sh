#!/bin/sh

set -xe

rm -rf /tmp/deb-package
cp -r deb-package /tmp
unzip "$1" -d /tmp/deb-package
rm -rf /tmp/deb-package/Applications/ESManager.app
mv -f /tmp/deb-package/Payload/ESManager.app /tmp/deb-package/Applications/
rm -rf /tmp/deb-package/Payload
find /tmp/deb-package/Applications -type f -exec md5sum '{}' ';' | sed -e 's;/tmp/deb-package;;' > /tmp/deb-package/DEBIAN/md5sums

VERS=`cat /tmp/deb-package/DEBIAN/control | grep 'Version:' | sed -e 's/Version: //'`

fakeroot dpkg-deb -Zgzip --build /tmp/deb-package /tmp/org.salieff.esmodinstaller_"$VERS"_iphoneos-arm.deb
