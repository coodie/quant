#include "Compressor.hpp"
#include "ProgramParameters.hpp"
#include "Debug.hpp"
#include <VectorOperations.hpp>

typedef float vecType;
typedef std::vector<vecType> vec;

namespace
{
}

namespace
{
  std::vector<vec> getVectorizedBlocks(const RGBImage& image, int w, int h)
  {
    const std::vector<int> &img = image.img;
    const int &xSize = image.xSize;
    const int &ySize = image.ySize;

    std::vector<vec> res;
    for(int i = 0; i < xSize/w; i++)
      for(int j = 0; j < ySize/h; j++)
      {
        vec tmp;
        tmp.reserve(w*h);

        for(int x = i*w; x < i*w+w; x++)
          for(int y = j*h; y < j*h+h; y++)
          {
            size_t index = x*xSize + y;
            if(index < img.size())
              tmp.push_back((vecType)img[index]);
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
  std::vector<vec> blocks = getVectorizedBlocks(image, par->width, par->height);
  std::vector<vec*> assignedCodeVector(blocks.size());

  std::vector<vec> c(1);
  c.reserve(1 << par->n);
  c[0].resize(blocks[0].size());

  // First vector is average
  for(auto &v : blocks)
    c[0] += v/(vecType)blocks.size();



  return CompressedImage();
}

