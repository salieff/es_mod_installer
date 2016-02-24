#!/bin/bash

set -xe

declare -a ICONS_ARR=( 
    'AppIcon29x29.png' '29x29' \
    'AppIcon29x29@2x.png' '58x58' \
    'AppIcon29x29@2x~ipad.png' '58x58' \
    'AppIcon29x29~ipad.png' '29x29' \
    'AppIcon40x40@2x.png' '80x80' \
    'AppIcon40x40@2x~ipad.png' '80x80' \
    'AppIcon40x40~ipad.png' '40x40' \
    'AppIcon50x50@2x~ipad.png' '100x100' \
    'AppIcon50x50~ipad.png' '50x50' \
    'AppIcon57x57.png' '57x57' \
    'AppIcon57x57@2x.png' '114x114' \
    'AppIcon60x60@2x.png' '120x120' \
    'AppIcon72x72@2x~ipad.png' '144x144' \
    'AppIcon72x72~ipad.png' '72x72' \
    'AppIcon76x76@2x~ipad.png' '152x152' \
    'AppIcon76x76~ipad.png' '76x76' )

declare -a LAUNCH_ARR=( \
    'LaunchImage-iOS7-568h@2x.png' '640x1136' \
    'LaunchImage-iOS7-Landscape.png' '1024x768' \
    'LaunchImage-iOS7-Landscape@2x.png' '2048x1536' \
    'LaunchImage-iOS7-Portrait.png' '768x1024' \
    'LaunchImage-iOS7-Portrait@2x.png' '1536x2048' \
    'LaunchImage-iOS7@2x.png' '640x960' \
    'LaunchImage.png' '320x480' \
    'LaunchImage@2x.png' '640x960' \
    'LaunchImage-568h@2x.png' '640x1136' \
    'LaunchImage-Landscape.png' '1024x748' \
    'LaunchImage-Landscape@2x.png' '2048x1496' \
    'LaunchImage-Portrait.png' '768x1004' \
    'LaunchImage-Portrait@2x.png' '1536x2008' )

IN_ICON='icon.png'

function processIcon() {
while (( "$#" ))
do
    local outName="$1"
    shift

    local outSize="$1"
    shift

    convert "${IN_ICON}" -resize "$outSize" "$outName"
done
}

function processLaunch() {
while (( "$#" ))
do
    local outName="$1"
    shift

    local outSizeX=`echo "$1" | perl -pe 's/x\d+$//'`
    local outSizeY=`echo "$1" | perl -pe 's/^\d+x//'`
    shift

    local inSizeX=`identify -format '%w' "${IN_ICON}"`
    local inSizeY=`identify -format '%h' "${IN_ICON}"`

    if [ "$inSizeX" -gt "$outSizeX" -o "$inSizeY" -gt "$outSizeY" ]; then
        convert "${IN_ICON}" -resize "${outSizeX}x${outSizeY}" "$outName"
    else
        cp "${IN_ICON}" "$outName"
    fi

    convert "$outName" -gravity center -background black -extent "${outSizeX}x${outSizeY}" "$outName"
done
}

processIcon "${ICONS_ARR[@]}"
processLaunch "${LAUNCH_ARR[@]}"
