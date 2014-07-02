#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/umbrella-ltc.png
ICON_DST=../../src/qt/res/icons/umbrella-ltc.ico
convert ${ICON_SRC} -resize 16x16 umbrella-ltc-16.png
convert ${ICON_SRC} -resize 32x32 umbrella-ltc-32.png
convert ${ICON_SRC} -resize 48x48 umbrella-ltc-48.png
convert umbrella-ltc-16.png umbrella-ltc-32.png umbrella-ltc-48.png ${ICON_DST}

