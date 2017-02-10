#!/bin/bash

set -xe

declare myDir=`dirname "$0"`
myDir=`cd "${myDir}"; pwd`

if [ "$#" -lt 3 ]; then
    echo "Usage: $0 MAJOR MINOR BUILD" 1>&2
    exit 1
fi

declare -i newMajor="$1"
declare -i newMinor="$2"
declare -i newBuild="$3"

perl -i -pe 's/^\s*#define\s+ESM_VERSION_MAJOR\s+\d+\s*/#define ESM_VERSION_MAJOR '"${newMajor}"'\n/g' "${myDir}/version.h"
perl -i -pe 's/^\s*#define\s+ESM_VERSION_MINOR\s+\d+\s*/#define ESM_VERSION_MINOR '"${newMinor}"'\n/g' "${myDir}/version.h"
perl -i -pe 's/^\s*#define\s+ESM_VERSION_BUILD\s+\d+\s*/#define ESM_VERSION_BUILD '"${newBuild}"'\n/g' "${myDir}/version.h"

perl -i -pe 's/android:versionName="[\d\.]+"/android:versionName="'"${newMajor}\.${newMinor}"'"/g' "${myDir}/android/AndroidManifest.xml"
perl -i -pe 's/android:versionCode="\d+"/android:versionCode="'"${newMajor}${newMinor}"'"/g' "${myDir}/android/AndroidManifest.xml"

perl -i -0pe 's/<key>CFBundleShortVersionString<\/key>\s*\n(\s*)<string>[\d\.]+<\/string>/<key>CFBundleShortVersionString<\/key>\n$1<string>'"${newMajor}\.${newMinor}"'<\/string>/g' "${myDir}/ESInfo.plist"
perl -i -0pe 's/<key>CFBundleVersion<\/key>\s*\n(\s*)<string>[\d\.]+<\/string>/<key>CFBundleVersion<\/key>\n$1<string>'"${newMajor}\.${newMinor}\.${newBuild}"'<\/string>/g' "${myDir}/ESInfo.plist"

perl -i -pe 's/Version: [\d\.-]+/Version: '"${newMajor}\.${newMinor}-${newBuild}"'/g' "${myDir}/deb-package/DEBIAN/control"

exit 0
