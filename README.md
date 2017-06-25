# quant
Quantization based lossy data compression

This is implementation of VQ algorithm as in [http://www.data-compression.com/vq.shtml](http://www.data-compression.com/vq.shtml).

Here is another nice link explaining the concept [http://www.gamasutra.com/view/feature/131499/image_compression_with_vector_.php](http://www.gamasutra.com/view/feature/131499/image_compression_with_vector_.php)

## Examples

| Before compression                                                 | After compression                                                             |
|--------------------------------------------------------------------|-------------------------------------------------------------------------------|
| ![](https://github.com/coodie/quant/blob/master/images/images/earth.png)  | ![](https://github.com/coodie/quant/blob/master/images/compressed_images/earth.png)  |
| 488K, PNG | 92K, Quant, 2,9 bits per pixel   |
| ![](https://github.com/coodie/quant/blob/master/images/images/splash.png) | ![](https://github.com/coodie/quant/blob/master/images/compressed_images/splash.png) |
| 388K, PNG | 92K, Quant, 2,9 bits per pixel   |
| ![](https://github.com/coodie/quant/blob/master/images/images/couple.png) | ![](https://github.com/coodie/quant/blob/master/images/compressed_images/couple.png) |
| 108K, PNG | 32K, Quant, 4 bits per pixel   |
| ![](https://github.com/coodie/quant/blob/master/images/images/beans.png)  | ![](https://github.com/coodie/quant/blob/master/images/compressed_images/beans.png)  |
| 84K, PNG | 32K, Quant, 4 bits per pixel   |

All of the above images have compression ratio between 15% and 20% compared to 24-bit per pixel image, and 1:2,5 to 1:5 ratio compared to PNG. Above images are being run with 2x2 block and 1024 codevectors, since experimental results showed that these give reasonable compression ratio, time and image quality.


## Building
Make sure you have `cmake`, `gcc` and `boost libraries` installed:

```
sudo apt-get install cmake gcc libboost-all-dev
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
