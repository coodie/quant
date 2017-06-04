#pragma once
#include <string>
#include <vector>
#include <memory>

const static int MAX_COL_BITS = 8;
const static int MAX_COL = 1 << MAX_COL_BITS;

class RGBImage
{
 public: 
  RGBImage() = default;
  RGBImage(const std::string &path);
  void saveToFile(const std::string &path);
  int sizeInBytes();
  std::vector<char> img;
  //sizes are given in pixels
  //size of img is = xSize*3*ySize*3
  int xSize, ySize;
};

