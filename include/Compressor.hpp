#pragma once
#include "RGBImage.hpp"
#include "VectorOperations.hpp"
#include <chrono>

class CompressionRaport
{
public:
  vecType distortion;
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
  friend std::pair<CompressedImage, CompressionRaport> compress(const RGBImage&, int blockWidth, int blockHeight, vecType eps, int N);
  friend RGBImage decompress(const CompressedImage&);

private:
  std::vector<vec> codeVectors;
  std::vector<size_t> assignedCodeVector;
  size_t xSize, ySize;
  size_t blockWidth, blockHeight;
};

std::pair<CompressedImage, CompressionRaport> compress(const RGBImage&, int blockWidth, int blockHeight, vecType eps, int N);
RGBImage decompress(const CompressedImage&);


vecType scale(char x);
char unscale(vecType x);
std::vector<vec> getBlocksAsVectorsFromImage(const RGBImage& image, int w, int h);
RGBImage getImageFromVectors(const std::vector<vec> &blocks, int xSize, int ySize, int w, int h);
std::tuple<std::vector<vec>, std::vector<size_t>, vecType> quantize(const std::vector<vec> &trainingSet, size_t n, vecType eps);
