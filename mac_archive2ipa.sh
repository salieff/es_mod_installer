#!/bin/sh

tmstamp=`date +'%Y-%m-%d_%T'`
outname="$HOME/Desktop/ESManager-$tmstamp.ipa"

rm -f "$outname"

xcodebuild -exportArchive -exportFormat ipa -archivePath "$1" -exportPath "$outname" && \
scp "$outname" salieff@salieff-n56vz:/tmp/

