#!/bin/bash

input_directory=$1
output_directory=$2

repo_link="https://github.com/coodie/quant/blob/master/images"

function echo_row()
{
    uncompressed_image=$1
    compressed_image=$2
    echo "| ![]($repo_link/$uncompressed_image)  | ![]($repo_link/$compressed_image)  |"
}

echo "| Before compression                                                 | After compression                                                             |"
echo "|--------------------------------------------------------------------|-------------------------------------------------------------------------------|"

for image in `ls $input_directory`
do
    echo_row "$input_directory/$image" "$output_directory/$image"
done
