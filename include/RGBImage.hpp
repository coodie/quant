#pragma once
#include <string>
#include <vector>
#include <memory>
#include <tuple>

const static int MAX_COL_BITS = 8;
const static int MAX_COL = 1 << MAX_COL_BITS;

#define RGBRange {0, 1, 2}
typedef std::array<char, 3> RGB;
typedef std::array<double, 3> RGBDouble;

class RGBImage
{
 public:
  RGBImage() = default;
  RGBImage(const std::string &path);
  void saveToFile(const std::string &path);
  size_t sizeInBytes() const;
  std::vector<RGB> img;
  int xSize, ySize;
};
