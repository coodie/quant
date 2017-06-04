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
  std::vector<int> img;
  int xSize, ySize;
};

