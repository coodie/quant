#include "RGBImage.hpp"
#include <fstream>
#include <cassert>
#include <iostream>

RGBImage::RGBImage(const std::string &path)
{
  std::fstream fs;
  fs.open(path, std::fstream::in | std::fstream::binary);
  std::string magic;
  fs >> magic;
  assert(magic == "P6");

  int x, y, max_col;

  fs >> x >> y >> max_col;
  fs.get();
  xSize = x;
  ySize = y;
  img.resize(x*y*3);
  assert(max_col == MAX_COL-1);

  fs.read(img.data(), img.size());
  fs.close();
}

void RGBImage::saveToFile(const std::string &path)
{
  std::fstream fs;
  fs.open(path, std::fstream::out | std::fstream::trunc);
  fs << "P6\n" << xSize << " " << ySize << "\n" << MAX_COL-1 << "\n";
  fs.write(img.data(),img.size());
  fs.flush();
  fs.close();
}

int RGBImage::sizeInBytes()
{
  return img.size();
}
