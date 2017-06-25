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
  img.resize(x*y);
  assert(max_col == MAX_COL-1);

  fs.read((char*)img.data(), img.size()*3);
  fs.close();
}

void RGBImage::saveToFile(const std::string &path)
{
  std::fstream fs;
  fs.open(path, std::fstream::out | std::fstream::trunc);
  fs << "P6\n" << xSize << " " << ySize << "\n" << MAX_COL-1 << "\n";
  fs.write((char*)img.data(),img.size()*3);
  fs.flush();
  fs.close();
}

size_t RGBImage::sizeInBytes() const
{
  return img.size()*3;
}
