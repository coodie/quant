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

## Algorithm details

In order to get yourself familiarized with this algorithm description I recommend reading articles: [http://www.data-compression.com/vq.shtml](http://www.data-compression.com/vq.shtml), [http://www.gamasutra.com/view/feature/131499/image_compression_with_vector_.php](http://www.gamasutra.com/view/feature/131499/image_compression_with_vector_.php).

Algorithm splits image into blocks of given size (default is 2x2) and then converts the blocks to vectors. Every pixel is considered to be 3x1 vector, and blocks are based on pixels, so 2x2 block (4 pixels) is transformed to 12 dimensional vector. Pixel values range from [0, 255], and this is scaled to [0, 1] range (this gives slightly better results), so in the end we receive 12-dimensional vector. 

Splitting method is used to find codevector. I start with one codevector being average of all vectors and in each iteration I run LBG algorithm to improve the codevector base, after LBG run number of codevectors is doubled with first half being multiplied by (1+eps) and the other by (1-eps), and then again LBG is being executed on these. We repeat splitting phase until desired number of codevectors is achieved.

The problem of empty codevector region is solved by assigning random vector from area of the biggest distortion.

## Possible improvements

There are numerous possible improvements I haven't been able to solve reasonably due to lack of time, experiments etc.
- Images contain a lot of artifacts. I saw improvements in artifacts reduction by modifying eps parameter, adding more codevectors, reducing block size, extending number of iterations in LBG algorithm, different approach to empty regions problem.
- Algorithm is slow. This one will be very hard to achieve since LBG is normally time consuming. Parallelization is highly non-obvious. Here are references which might help in parallelizing: https://arxiv.org/abs/0910.4711, http://ieeexplore.ieee.org/document/1402243/.
- Change color space from RGB to https://en.wikipedia.org/wiki/Lab_color_space
- Use lossless compression methods after quantization run: Huffman encoding, LZ family, possibly others.
- Change algorithm completely. NeuQuant seems to be far more interesting than LBG: https://scientificgems.wordpress.com/stuff/neuquant-fast-high-quality-image-quantization/ and has far better results.

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
