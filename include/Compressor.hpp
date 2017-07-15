#pragma once
#include "VectorOperations.hpp"
#include "ColorSpace.hpp"
#include <chrono>
#include <memory>

class CompressionRaport
{
public:
  vecType distortion;
  float bitsPerPixel;
  size_t uncompressedSize;
  size_t compressedSize;
  std::chrono::duration<double> compressionTime;
  friend std::ostream& operator <<(std::ostream& stream, const CompressionRaport& raport);
};

enum class Quantizers
{
  LBG, MEDIAN_CUT, LBG_MEDIAN_CUT
};

class AbstractQuantizer
{
public:
  virtual std::tuple<std::vector<vec>, std::vector<size_t>, vecType> quantize(const std::vector<vec> &trainingSet, size_t n, vecType eps) = 0;
  virtual ~AbstractQuantizer() = default;
};

typedef std::unique_ptr<AbstractQuantizer> QuantizerPtr;

QuantizerPtr getQuantizer(Quantizers);

class CompressedImage
{
public:
  CompressedImage() = default;
  void saveToFile(const std::string &path);
  void loadFromFile(const std::string &path);
  size_t sizeInBits();
  friend std::pair<CompressedImage, CompressionRaport> compress(const RGBImage&, const QuantizerPtr &quantizer, const ColorSpacePtr &colorSpace, int blockWidth, int blockHeight, vecType eps, int N);
  friend RGBImage decompress(const CompressedImage&, const ColorSpacePtr&);

private:
  std::vector<vec> codeVectors;
  std::vector<size_t> assignedCodeVector;
  size_t xSize, ySize;
  size_t blockWidth, blockHeight;
};

std::pair<CompressedImage, CompressionRaport> compress(const RGBImage&, const QuantizerPtr &quantizer, const ColorSpacePtr &colorSpace, int blockWidth, int blockHeight, vecType eps, int N);
RGBImage decompress(const CompressedImage&, const ColorSpacePtr&);

std::vector<vec> getBlocksAsVectorsFromImage(const RGBImage& image, int w, int h, const ColorSpacePtr&);
RGBImage getImageFromVectors(const std::vector<vec> &blocks, int xSize, int ySize, int w, int h, const ColorSpacePtr&);
std::tuple<std::vector<vec>, std::vector<size_t>, vecType> quantize(const std::vector<vec> &trainingSet, size_t n, vecType eps);
