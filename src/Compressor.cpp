#include "Compressor.hpp"
#include "ProgramParameters.hpp"
#include "Debug.hpp"

#include <vector>

namespace
{
  std::vector<std::vector<float>> getVectorizedBlocks(const RGBImage& image, int w, int h)
  {
    const std::vector<int> &img = image.img;
    const int &xSize = image.xSize;
    const int &ySize = image.ySize;

    std::vector<std::vector<float>> res;
    for(int i = 0; i < xSize/w; i++)
      for(int j = 0; j < ySize/h; j++)
      {
        std::vector<float> tmp;
        tmp.reserve(w*h);

        for(int x = i*w; x < i*w+w; x++)
          for(int y = j*h; y < j*h+h; y++)
          {
            size_t index = x*xSize + y;
            if(index < img.size())
              tmp.push_back((float)img[index]);
            else
              tmp.push_back(0.0);
          }
        res.push_back(std::move(tmp));
      }
    return res;
  }
}

CompressedImage compress(const RGBImage& image)
{
  ProgramParameters *par = getParams();
  std::vector<std::vector<float>> blocks = getVectorizedBlocks(image, par->width, par->height);

  DEBUG_PRINT(blocks);

  return CompressedImage();
}

