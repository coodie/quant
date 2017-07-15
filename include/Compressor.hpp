#pragma once
#include "VectorOperations.hpp"
#include "ColorSpace.hpp"
#include "Quantizer.hpp"

#include <chrono>
#include <memory>

class CompressionRaport
{
public:
  VectorType distortion;
  float bitsPerPixel;
  size_t uncompressedSize;
  size_t compressedSize;
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
  friend std::pair<CompressedImage, CompressionRaport> compress(const RGBImage&, const QuantizerPtr &quantizer, const ColorSpacePtr &colorSpace, int blockWidth, int blockHeight, VectorType eps, int N);
  friend RGBImage decompress(const CompressedImage&, const ColorSpacePtr&);

private:
  std::vector<Vector> codeVectors;
  std::vector<size_t> assignedCodeVector;
  size_t xSize, ySize;
  size_t blockWidth, blockHeight;
};

std::pair<CompressedImage, CompressionRaport> compress(const RGBImage&, const QuantizerPtr &quantizer, const ColorSpacePtr &colorSpace, int blockWidth, int blockHeight, VectorType eps, int N);
RGBImage decompress(const CompressedImage&, const ColorSpacePtr&);

std::vector<Vector> getBlocksAsVectorsFromImage(const RGBImage& image, int w, int h, const ColorSpacePtr&);
RGBImage getImageFromVectors(const std::vector<Vector> &blocks, int xSize, int ySize, int w, int h, const ColorSpacePtr&);
std::tuple<std::vector<Vector>, std::vector<size_t>, VectorType> quantize(const std::vector<Vector> &trainingSet, size_t n, VectorType eps);
