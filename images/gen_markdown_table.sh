#!/bin/bash

uncompressed_directory=$1
compressed_directory=$2

repo_link="https://github.com/coodie/quant/blob/master/images"

function echo_row()
{
    uncompressed_image=$1
    compressed_image=$2
    raport_file=$3
    echo "| ![]($repo_link/$uncompressed_image)  | ![]($repo_link/$compressed_image)  |"

    png_size=`du -h $uncompressed_image | cut -f1`
    quant_size=`cat $raport_file | grep "Compressed size" | cut -d= -f2`
    bits_per_pixel=`cat $raport_file | grep "Bits per pixel" | cut -d= -f2`
    distortion=`cat $raport_file | grep "Distortion" | cut -d= -f2`

    echo "| PNG: $png_size  | quant: $quant_size, bits per pixel: $bits_per_pixel, distortion: $distortion |"
}

echo "| Before compression                                                 | After compression                                                             |"
echo "|--------------------------------------------------------------------|-------------------------------------------------------------------------------|"

for image in `ls $uncompressed_directory`
do
    raport_file="$compressed_directory/${image%.*}.raport"
    echo_row "$uncompressed_directory/$image" "$compressed_directory/$image" $raport_file
done
