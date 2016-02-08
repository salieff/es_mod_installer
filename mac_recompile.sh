#!/bin/sh

rm -rf /tmp/ESManager.xcarchive

xcodebuild clean -project ESManager.xcodeproj -configuration Release -alltargets && \
xcodebuild archive -project ESManager.xcodeproj -configuration Release -scheme ESManager -archivePath /tmp/ESManager.xcarchive && \
./mac_archive2ipa.sh /tmp/ESManager.xcarchive

