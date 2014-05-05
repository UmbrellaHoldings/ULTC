#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/xxxxxxx.png
ICON_DST=../../src/qt/res/icons/xxxxxxx.ico
convert ${ICON_SRC} -resize 16x16 xxxxxxx-16.png
convert ${ICON_SRC} -resize 32x32 xxxxxxx-32.png
convert ${ICON_SRC} -resize 48x48 xxxxxxx-48.png
convert xxxxxxx-16.png xxxxxxx-32.png xxxxxxx-48.png ${ICON_DST}

