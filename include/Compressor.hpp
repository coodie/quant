#pragma once
#include "RGBImage.h"

class CompressedImage
{
public: 
  CompressedImage() = default;
  void saveToFile(const std::string &path);
  int sizeInBytes();
private:
  std::vector<int> img;
  int xSize, ySize;
};

CompressedImage compress(const RGBImage&);

