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
  static std::pair<CompressedImage, CompressionRaport> compress(const RGBImage&, const QuantizerPtr &quantizer, const ColorSpacePtr &colorSpace, int blockWidth, int blockHeight, VectorType eps, int N);
  static RGBImage decompress(const CompressedImage&, const ColorSpacePtr&);

private:
  std::vector<CharVector> codeVectors;
  std::vector<size_t> assignedCodeVector;
  size_t xSize, ySize;
  size_t blockWidth, blockHeight;
};

std::vector<CharVector> vectorsToCharVectorsColorSpaced(const std::vector<Vector>& vectors, const ColorSpacePtr &cs);
std::vector<Vector> getBlocksAsVectorsFromImage(const RGBImage& image, int w, int h, const ColorSpacePtr&);

RGBImage getImageFromVectors(const std::vector<CharVector> &blocks, int xSize, int ySize, int w, int h);
std::tuple<std::vector<Vector>, std::vector<size_t>, VectorType> quantize(const std::vector<Vector> &trainingSet, size_t n, VectorType eps);
