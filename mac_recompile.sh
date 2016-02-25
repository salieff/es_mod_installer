#!/bin/sh

MY_DIR=`dirname "$0"`
MY_DIR=`cd "$MY_DIR"; pwd`

rm -rf /tmp/ESManager.xcarchive

xcodebuild clean -project ESManager.xcodeproj -configuration Release -alltargets && \
xcodebuild archive -project ESManager.xcodeproj -configuration Release -scheme ESManager -archivePath /tmp/ESManager.xcarchive CODE_SIGN_IDENTITY="-" && \
"$MY_DIR/mac_archive2ipa.sh" /tmp/ESManager.xcarchive

