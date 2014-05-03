#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/siliconvalley.png
ICON_DST=../../src/qt/res/icons/siliconvalley.ico
convert ${ICON_SRC} -resize 16x16 siliconvalley-16.png
convert ${ICON_SRC} -resize 32x32 siliconvalley-32.png
convert ${ICON_SRC} -resize 48x48 siliconvalley-48.png
convert siliconvalley-16.png siliconvalley-32.png siliconvalley-48.png ${ICON_DST}

