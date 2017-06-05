# quant
Quantization based lossless data compression

This is implementation of VQ algorithm as in [http://www.data-compression.com/vq.shtml](link).
## Initial results

| Before compression                                                 | After compression                                                             |
|--------------------------------------------------------------------|-------------------------------------------------------------------------------|
| ![](https://github.com/coodie/quant/blob/master/images/earth.png)  | ![](https://github.com/coodie/quant/blob/master/images/earth_compressed.png)  |
| ![](https://github.com/coodie/quant/blob/master/images/splash.png) | ![](https://github.com/coodie/quant/blob/master/images/splash_compressed.png) |
| ![](https://github.com/coodie/quant/blob/master/images/couple.png) | ![](https://github.com/coodie/quant/blob/master/images/couple_compressed.png) |
| ![](https://github.com/coodie/quant/blob/master/images/beans.png)  | ![](https://github.com/coodie/quant/blob/master/images/beans_compressed.png)  |
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

Once you have your `.ppm` file here is quick way to use your program:
```
quant --file input.ppm -o output.ppm 
```

For more options (playing with parameters) use:
```
quant --help
```
