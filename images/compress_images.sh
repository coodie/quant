#/bin/bash

function compress_image()
{
    local in=$1
    local out=$2

    echo "Compressing image $in"

    in="${in%.*}"
    out="${out%.*}"

    pngtopnm $in.png > $in.ppm

    ../build/quant -n 8 --file $in.ppm -o $out.ppm -h 1 -w 1 | tee $out.raport

    pnmtopng $out.ppm > $out.png

    rm $in.ppm
    rm $out.ppm
}

#for image in `ls images`
#do
#    compress_image "images/$image" "compressed_images/$image"
#done

for image in `ls kodim`
do
    compress_image "kodim/$image" "compressed_kodim_small_block/$image"
done
