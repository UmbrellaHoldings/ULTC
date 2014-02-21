#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/vertcoin.png
ICON_DST=../../src/qt/res/icons/vertcoin.ico
convert ${ICON_SRC} -resize 16x16 vertcoin-16.png
convert ${ICON_SRC} -resize 32x32 vertcoin-32.png
convert ${ICON_SRC} -resize 48x48 vertcoin-48.png
convert vertcoin-16.png vertcoin-32.png vertcoin-48.png ${ICON_DST}

