#include "RGBImage.h"
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
  img.reserve(x*y);

  assert(max_col == MAX_COL-1);
  for(int i = 0; i < x*y; i++)
  {
    char r[3];
    fs.read(r, 3);
    int data = (r[0]*MAX_COL*MAX_COL) + (r[1]*MAX_COL) + r[2];
    img.push_back(data);
  }
  fs.close();
}

void RGBImage::saveToFile(const std::string &path)
{
  std::fstream fs;
  fs.open(path, std::fstream::out | std::fstream::trunc);
  fs << "P6\n" << xSize << " " << ySize << "\n" << MAX_COL-1 << "\n";
  for(auto val : img)
  {
    char r[3];
    r[0] = val/(MAX_COL*MAX_COL);
    r[1] = (val/MAX_COL) % MAX_COL;
    r[2] = val % MAX_COL;
    fs.write(r,3);
  }
  fs.flush();
  fs.close();
}

int RGBImage::sizeInBytes()
{
  return img.size()*3;
}
