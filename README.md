# quant
Quantization based lossless data compression

This is implementation of VQ algorithm as in [http://www.data-compression.com/vq.shtml](link).

## Building
Make sure you have `cmake`, `gcc` and `boost libraries` installed:

```
sudo apt-get cmake gcc install libboost-all-dev
```

Pick a directory and clone into it using:
```
git clone https://github.com/coodie/quant
cd quant
```

Time to build the project:
```
mkdir build
cd build
cmake ..
make -j
```


## Usage
Currently only `.ppm` file compression is supported. Best way to get `.ppm` out of your favourite format is to use Netpbm package, 
it is usualy installed on most of linux distros.
Here are examples how to get `.ppm` file from `.png` and `.jpg`.

```
pngtopnm file.png > file.ppm
jpegtopnm file.jpg > file.ppm
```
