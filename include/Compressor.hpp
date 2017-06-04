#pragma once
#include "RGBImage.hpp"

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

RGBImage compress(const RGBImage&);
