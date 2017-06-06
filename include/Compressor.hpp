#pragma once
#include "RGBImage.hpp"
#include "VectorOperations.hpp"
#include <chrono>

class CompressionRaport
{
public:
  float distortion;
  float bitsPerPixel;
  std::chrono::duration<double> compressionTime;
  friend std::ostream& operator <<(std::ostream& stream, const CompressionRaport& raport);
};

class CompressedImage
{
public: 
  CompressedImage() = default;
  void saveToFile(const std::string &path);
  void loadFromFile(const std::string &path);
  size_t sizeInBits();
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
