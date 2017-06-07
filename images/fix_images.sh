#/bin/bash

function fix_image()
{
    local in=$1
    local out=$2
    ../build/quant --file $in -o $out -w 1 -h 1
    local in_png="${in%.*}.png"
    local out_png="${out%.*}.png"

    pnmtopng $out > $out_png
}

fix_image earth.ppm earth_compressed.ppm
fix_image splash.ppm splash_compressed.ppm
fix_image couple.ppm couple_compressed.ppm
fix_image beans.ppm beans_compressed.ppm
