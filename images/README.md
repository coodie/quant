# Testing Images

This directory contains images used to test Quant.
- `images` - random example images taken from internet
- `kodim` - kodim images test suite

## Running tests
In order to run quant on every image, simply run `compress_images.sh` script, provided that quant executable is in `ROOT/build/`, where ROOT is root directory of this project. Keep in mind that this might take a while.

## Generating table
In order to generate nicely rendered table after running `compress_image.sh` script simply execute `gen_markdown_table.sh uncompressed_directory compressed_directory`.
