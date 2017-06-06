#include "Compressor.hpp"
#include "ProgramParameters.hpp"
#include "Debug.hpp"
#include "KDTree.hpp"
#include "VectorOperations.hpp"

#include <fstream>
#include <chrono>

namespace
{
  std::vector<vec> getBlocksAsVectorsFromImage(const RGBImage& image, int w, int h)
  {
    const std::vector<char> &img = image.img;

    const int &xSize = image.xSize*3;
    const int &ySize = image.ySize;

    size_t wBlocks = (xSize+w-1)/w;
    size_t hBlocks = (ySize+h-1)/h;

    std::vector<vec> res(wBlocks*hBlocks);

    for(size_t i = 0; i < wBlocks; i++)
      for(size_t j = 0; j < hBlocks; j++)
      {
        vec tmp(w*h);
        for(size_t x = i*w; x < i*w+w; x++)
          for(size_t y = j*h; y < j*h+h; y++)
          {
            size_t imgIndex = x*ySize + y;
            size_t vecIndex = (x-i*w)*h+(y-j*h);
            if(imgIndex < img.size())
              tmp.at(vecIndex) = (float)img.at(imgIndex);
            else
              tmp.at(vecIndex) = 0.0;
          }
        res.at(i*hBlocks+j) = std::move(tmp);
      }
    return res;
  }

  RGBImage getImageFromVectors(const std::vector<vec> &blocks, int xSize, int ySize, int w, int h)
  {
    RGBImage res;
    xSize *= 3;
    std::vector<char> imgData(xSize*ySize);

    size_t wBlocks = (xSize+w-1)/w;
    size_t hBlocks = (ySize+h-1)/h;

    for(size_t i = 0; i < wBlocks; i++)
      for(size_t j = 0; j < hBlocks; j++)
        {
          const vec &tmp = blocks.at(i*hBlocks+j);

          for(size_t x = i*w; x < i*w+w; x++)
            for(size_t y = j*h; y < j*h+h; y++)
              {
                size_t imgIndex = x*ySize + y;
                size_t vecIndex = (x-i*w)*h+(y-j*h);
                if(imgIndex < imgData.size())
                  imgData.at(imgIndex) = (char)tmp.at(vecIndex);
              }
        }

    res.img = std::move(imgData);
    res.xSize = xSize/3;
    res.ySize = ySize;
    return res;
  }



  vecType getDistortion(const std::vector<vec> &trainingSet,
                        const std::vector<size_t> &assignedCodeVector,
                        const std::vector<vec> &codeVectors)
  {
    size_t dim = trainingSet[0].size();
    size_t M = trainingSet.size();

    vecType res = 0;
    for(size_t i = 0; i < trainingSet.size(); i++)
    {
      const auto &x = trainingSet[i];
      const auto &c = codeVectors[assignedCodeVector[i]];
      res += inner_product(x-c);
    }
    return res/((vecType)(M*dim));
  }

  template<typename T>
  void removeMarked(std::vector<T> &v, const std::vector<bool> &marked)
  {
    int last = 0;
    for(int i = 0; i < v.size(); ++i, ++last)
      {
        while(marked[i])
          ++i;
        if(i >= v.size()) break;
        v[last] = v[i];
      }

    v.resize(last);
  }

  void assignCodeVectors(const std::vector<vec> &trainingSet, const std::vector<vec> &codeVectors, std::vector<size_t> &assignedCodeVector)
  {
    size_t dim = trainingSet.front().size();
    KDTree kdtree(dim, codeVectors);

    for(size_t i = 0; i < trainingSet.size(); i++)
    {
      size_t found = kdtree.nearestNeighbour(trainingSet[i]);
      assignedCodeVector[i] = found;
    }
  }

  const int MAX_IT = 3;

  std::tuple<std::vector<vec>, std::vector<size_t>, vecType> quantize(const std::vector<vec> &trainingSet, size_t n, vecType eps)
  {
    size_t dim = trainingSet[0].size();
    size_t M = trainingSet.size();
    size_t maxCodeVectors = 1 << n;

    std::vector<size_t> assignedCodeVector(trainingSet.size());
    std::vector<vec> codeVectors(1, vec(dim));
    codeVectors.reserve(maxCodeVectors);

    // Initialize values
    for(auto &v : trainingSet) codeVectors[0] += v/(vecType)M;
    for(auto &a : assignedCodeVector) a = 0;
    vecType distortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);

    while(codeVectors.size() < maxCodeVectors)
    {
      // splitting phase
      concat(codeVectors, codeVectors);
      for(size_t i = 0; i < codeVectors.size()/2; i++)
      {
        codeVectors[i] *= (vecType)(1+eps);
        codeVectors[i+codeVectors.size()/2] *= (vecType)(1-eps);
      }

      // iteration phase
      std::vector<vec> newCodeVectors;
      for(size_t it = 0; it < MAX_IT; it++)
      {
        assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);

        newCodeVectors = std::vector<vec>(codeVectors.size(), vec(dim));
        std::vector<size_t> assignedCounts(codeVectors.size());

        for(size_t i = 0; i < M; i++)
        {
          int cur = assignedCodeVector[i];
          const vec &x = trainingSet[i];

          newCodeVectors[cur] += x;
          assignedCounts[cur] ++;
        }

        std::vector<bool> removable(codeVectors.size(), false);

        for(size_t i = 0; i < newCodeVectors.size(); i++)
        {
          if(assignedCounts[i] != 0)
            newCodeVectors[i] /= (vecType)assignedCounts[i];
          else
            removable[i] = true;
        }

        removeMarked(newCodeVectors, removable);

        vecType newDistortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);
        if((distortion-newDistortion)/distortion > eps)
           break;
        distortion = newDistortion;
      }
      codeVectors = newCodeVectors;
    }

    assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);

    return std::make_tuple(codeVectors, assignedCodeVector, distortion);
  }
}

std::pair<CompressedImage, CompressionRaport> compress(const RGBImage& image)
{
  ProgramParameters *par = getParams();
  size_t blockWidth = par->width;
  size_t blockHeight = par->height;

  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  auto trainingSet = getBlocksAsVectorsFromImage(image, blockWidth, blockHeight);
  std::vector<vec> codeVectors;
  std::vector<size_t> assignedCodeVector;
  vecType distortion;
  std::tie(codeVectors, assignedCodeVector, distortion) = quantize(trainingSet, par->n, par->eps);

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> compressionTime = end-start;

  CompressedImage resImg;
  resImg.codeVectors = std::move(codeVectors);
  resImg.assignedCodeVector = std::move(assignedCodeVector);
  resImg.xSize = image.xSize;
  resImg.ySize = image.ySize;
  resImg.blockWidth = blockWidth;
  resImg.blockHeight = blockHeight;

  float bitsPerPixel = ((float)resImg.sizeInBits())/(image.xSize*image.ySize);
  CompressionRaport raport{distortion, bitsPerPixel, compressionTime};
  return std::make_pair(resImg, raport);
}


RGBImage decompress(const CompressedImage& cImg)
{
  std::vector<vec> quantizedTrainingSet(cImg.assignedCodeVector.size());
  for(size_t i = 0; i < quantizedTrainingSet.size(); i++)
    quantizedTrainingSet[i] = cImg.codeVectors[cImg.assignedCodeVector[i]];

  RGBImage quantizedImg = getImageFromVectors(quantizedTrainingSet, cImg.xSize, cImg.ySize, cImg.blockWidth, cImg.blockHeight);
  return quantizedImg;
}

size_t smallestPow2(size_t n)
{
  int p = 0;
  while(n/=2) p++;
  return 1 << p;
}

size_t CompressedImage::sizeInBits()
{
  // At this moment approximate size
  size_t codeVectorBits = smallestPow2(codeVectors.size()-1);
  size_t dimension = blockWidth*blockHeight;
  size_t bits = codeVectorBits*assignedCodeVector.size() + // Store image data
    dimension*codeVectors.size()*sizeof(char); // Store codevectors

  return ((bits+7)/8)*8; //align
}

void CompressedImage::saveToFile(const std::string &path)
{
  std::fstream file(path, std::fstream::out | std::fstream::trunc | std::fstream::binary);
  file << xSize << " " << ySize << " " << blockWidth << " " << blockHeight << " " << codeVectors.size();

  for(auto &cv : codeVectors) for(auto &c : cv) file << (char)c;
  for(auto &a : assignedCodeVector) file << a;
}

void CompressedImage::loadFromFile(const std::string &path)
{
  throw std::runtime_error("not implemented");

  std::fstream file(path, std::fstream::in | std::fstream::binary);
  size_t codeVectorsSize;
  file >> xSize >> ySize >> blockWidth >> blockHeight >> codeVectorsSize;

  codeVectors = std::vector<vec>(codeVectorsSize, vec(blockWidth*blockHeight));
  for(auto &cv : codeVectors) for(auto &c : cv) file >> c;
  for(auto &a : assignedCodeVector) file >> a;
}

std::ostream& operator <<(std::ostream& stream, const CompressionRaport& raport)
{
  stream << "Compression raport: " << std::endl;
  stream << "distortion       = " << raport.distortion << std::endl;
  stream << "bits per pixel   = " << raport.bitsPerPixel << std::endl;
  stream << "compression time = " << raport.compressionTime.count() << "s" << std::endl;
  return stream;
}

