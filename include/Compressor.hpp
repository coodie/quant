#pragma once
#include "RGBImage.hpp"
#include "VectorOperations.hpp"

class CompressionRaport
{
public:
  vecType distortion;
};

class CompressedImage
{
public: 
  CompressedImage() = default;
  void saveToFile(const std::string &path);
  void loadFromFile(const std::string &path);
  RGBImage convertToPPM();
  size_t sizeInBytes();
  friend std::pair<CompressedImage, CompressionRaport> compress(const RGBImage&);
  friend RGBImage decompress(const CompressedImage&);

private:
  std::vector<vec> codeVectors;
  std::vector<size_t> assignedCodeVector;
  size_t xSize, ySize;
  size_t blockWidth, blockHeight;
};

std::pair<CompressedImage, CompressionRaport> compress(const RGBImage&);
RGBImage decompress(const CompressedImage&);
