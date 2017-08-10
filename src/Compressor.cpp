#include "Compressor.hpp"
#include "Debug.hpp"
#include "KDTree.hpp"
#include "VectorOperations.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>

std::vector<CharVector> vectorsToCharVectorsColorSpaced(
    const std::vector<Vector> &vectors, const ColorSpacePtr &cs) {
  std::vector<CharVector> res;
  std::transform(
      begin(vectors), end(vectors), std::back_inserter(res),
      [&](const Vector &a) {
        CharVector res(a.size());
        for (size_t i = 0; i < a.size(); i += 3) {
          RGB unscaled = cs->colorSpaceToRGB({a[i], a[i + 1], a[i + 2]});
          res[i] = unscaled.at(0);
          res[i + 1] = unscaled.at(1);
          res[i + 2] = unscaled.at(2);
        }
        return res;
      });
  return res;
}

std::vector<Vector> getBlocksAsVectorsFromImage(const RGBImage &image, int w,
                                                int h,
                                                const ColorSpacePtr &cs) {
  const std::vector<RGB> &img = image.img;

  int xSize = image.xSize;
  int ySize = image.ySize;

  size_t wBlocks = (xSize + w - 1) / w;
  size_t hBlocks = (ySize + h - 1) / h;

  std::vector<Vector> res(wBlocks * hBlocks);

  for (size_t i = 0; i < wBlocks; i++)
    for (size_t j = 0; j < hBlocks; j++) {
      Vector tmp(3 * w * h);
      for (size_t x = i * w; x < i * w + w; x++)
        for (size_t y = j * h; y < j * h + h; y++) {
          size_t imgIndex = x * ySize + y;
          size_t vecIndex = ((x - i * w) * h + (y - j * h)) * 3;

          for (auto shift : RGBRange)
            if (imgIndex < img.size()) {
              RGBDouble scaled = cs->RGBtoColorSpace(img.at(imgIndex));
              tmp.at(vecIndex + shift) = scaled.at(shift);
            } else
              tmp.at(vecIndex + shift) = 0;
        }
      res.at(i * hBlocks + j) = std::move(tmp);
    }
  return res;
}

RGBImage getImageFromVectors(const std::vector<CharVector> &blocks, int xSize,
                             int ySize, int w, int h) {
  size_t wBlocks = (xSize + w - 1) / w;
  size_t hBlocks = (ySize + h - 1) / h;

  std::vector<RGB> imgData(xSize * ySize);

  for (size_t i = 0; i < wBlocks; i++)
    for (size_t j = 0; j < hBlocks; j++) {
      const CharVector &tmp = blocks.at(i * hBlocks + j);

      for (size_t x = i * w; x < i * w + w; x++)
        for (size_t y = j * h; y < j * h + h; y++) {
          size_t imgIndex = x * ySize + y;
          size_t vecIndex = ((x - i * w) * h + (y - j * h)) * 3;

          if (imgIndex < imgData.size()) {
            for (auto shift : RGBRange)
              imgData.at(imgIndex).at(shift) = tmp.at(vecIndex + shift);
          }
        }
    }

  RGBImage img;
  img.ySize = ySize;
  img.xSize = xSize;
  img.img = std::move(imgData);
  return img;
}

std::chrono::duration<double> measureExecutionTime(
    const std::function<void()> &f) {
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  f();

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> executionTime = end - start;

  return executionTime;
}

std::pair<CompressedImage, CompressionRaport> CompressedImage::compress(
    const RGBImage &image, const QuantizerPtr &quantizer,
    const ColorSpacePtr &colorSpace, int blockWidth, int blockHeight,
    VectorType eps, int N) {
  std::vector<Vector> codeVectors;
  std::vector<size_t> assignedCodeVector;
  VectorType distortion;

  auto compressionTime = measureExecutionTime([&]() {
    auto trainingSet =
        getBlocksAsVectorsFromImage(image, blockWidth, blockHeight, colorSpace);
    std::tie(codeVectors, assignedCodeVector, distortion) =
        quantizer->quantize(trainingSet, N, eps);
  });

  CompressedImage resImg;
  resImg.codeVectors = vectorsToCharVectorsColorSpaced(codeVectors, colorSpace);
  resImg.assignedCodeVector = std::move(assignedCodeVector);
  resImg.xSize = image.xSize;
  resImg.ySize = image.ySize;
  resImg.blockWidth = blockWidth;
  resImg.blockHeight = blockHeight;

  float bitsPerPixel =
      ((float)resImg.sizeInBits()) / (image.xSize * image.ySize);

  // Decompress image temporarily to compute real distortion
  {
    RGBImage img = decompress(resImg, colorSpace);
    distortion = 0;
    for (size_t i = 0; i < image.img.size(); i++) {
      for (auto j : RGBRange)
        distortion += std::pow(
            (VectorType)image.img[i][j] - (VectorType)img.img[i][j], 2);
    }
    distortion /= image.img.size() * 3;
  }

  size_t uncompressedSize = image.sizeInBytes();
  size_t compressedSize = resImg.sizeInBits() / 8;

  CompressionRaport raport{distortion, bitsPerPixel, uncompressedSize,
                           compressedSize, compressionTime};
  return std::make_pair(resImg, raport);
}

RGBImage CompressedImage::decompress(const CompressedImage &cImg,
                                     const ColorSpacePtr &cs) {
  std::vector<CharVector> quantizedTrainingSet(cImg.assignedCodeVector.size());
  for (size_t i = 0; i < quantizedTrainingSet.size(); i++)
    quantizedTrainingSet[i] = cImg.codeVectors[cImg.assignedCodeVector[i]];

  RGBImage res =
      getImageFromVectors(quantizedTrainingSet, cImg.xSize, cImg.ySize,
                          cImg.blockWidth, cImg.blockHeight);
  return res;
}

size_t smallestPow2(size_t n) {
  int p = 0;
  while (n /= 2) p++;
  return p;
}

size_t CompressedImage::sizeInBits() {
  // At this moment approximate size
  size_t codeVectorBits = smallestPow2(codeVectors.size());
  size_t dimension = blockWidth * blockHeight;
  size_t bits = codeVectorBits * assignedCodeVector.size() +
                dimension * codeVectors.size() * 8 * 3;  // Store codevectors

  return ((bits + 7) / 8) * 8;  // align
}

void writeBits(std::fstream &file, size_t data, size_t bits, char &prevByte,
               size_t &prevBits) {
  prevBits += bits;
  while (bits > 0) {
    char byte = prevByte;
  }
}

void CompressedImage::saveToFile(const std::string &path) {
  size_t bitsPerCodeVector = smallestPow2(assignedCodeVector.size());
  assert(bitsPerCodeVector <= 24);
  std::fstream file(path);
  file << bitsPerCodeVector << ' ' << assignedCodeVector.size() << ' ' << xSize
       << ' ' << ySize << ' ' << blockWidth << ' ' << blockHeight << std::endl;

  size_t codeVectorSize = blockWidth * blockHeight * 3;

  // Write codeVectors first
  assert((1 << bitsPerCodeVector) == codeVectors.size());
  for (size_t i = 0; i < codeVectors.size(); i++)
    for (size_t j = 0; j < codeVectorSize; j++) file << codeVectors[i][j];
  file << std::endl;

  bitsPerCodeVector = ((bitsPerCodeVector + 7) / 8) * 8;
  int bytesPerCodeVector = bitsPerCodeVector / 8;
  for (size_t i = 0; i < assignedCodeVector.size(); i++) {
    char *cur = reinterpret_cast<char *>(&assignedCodeVector[i]);
    for (size_t j = 0; j < bytesPerCodeVector; j++) file << cur[j];
  }
}

void CompressedImage::loadFromFile(const std::string &path) {
  size_t bitsPerCodeVector;
  std::fstream file(path);
  file >> bitsPerCodeVector >> xSize >> ySize >> blockWidth >> blockHeight;
  assert(bitsPerCodeVector <= 24);
  for (size_t i = 0; i < codeVectors.size(); i++)
    for (size_t j = 0; j < codeVectorSize; j++) file >> codeVectors[i][j];
  file << std::endl;

  throw std::runtime_error("not implemented");
}

std::string prettyPrintBytes(size_t bytes) {
  std::stringstream res;
  if (bytes < 1024) {
    res << bytes << "b";
    return res.str();
  }

  if (bytes < 1024 * 1024) {
    size_t kb = bytes / 1024;
    res << kb << "," << bytes % 1024 << "Kb";
    return res.str();
  }

  size_t mb = bytes / (1024 * 1024);
  size_t kb = bytes % (1024 * 1024);

  res << mb << "," << kb << "Mb";
  return res.str();
}

std::ostream &operator<<(std::ostream &stream,
                         const CompressionRaport &raport) {
  stream << "Compression raport: " << std::endl;
  stream << "Distortion        = " << std::fixed << std::setprecision(10)
         << raport.distortion << std::endl;
  stream << "Bits per pixel    = " << raport.bitsPerPixel << std::endl;
  stream << "Uncompressed size = " << prettyPrintBytes(raport.uncompressedSize)
         << std::endl;
  stream << "Compressed size   = " << prettyPrintBytes(raport.compressedSize)
         << std::endl;
  stream << "Compression ratio = " << std::fixed << std::setprecision(3)
         << (double)raport.compressedSize / raport.uncompressedSize
         << std::endl;
  stream << "Compression time  = " << raport.compressionTime.count() << "s"
         << std::endl;
  return stream;
}
