#include "Compressor.hpp"
#include "Debug.hpp"
#include "KDTree.hpp"
#include "VectorOperations.hpp"

#include <set>
#include <fstream>
#include <chrono>

namespace
{
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

  template<typename T, typename cmp>
  T extract_max(std::set<T, cmp> &q)
  {
    auto it = q.begin();
    T tmp(std::move(*it));
    q.erase(it);
    return tmp;
  }

  template<typename T, typename cmp>
  T extract_min(std::set<T, cmp> &q)
  {
    auto it = --(q.end());
    T tmp(std::move(*it));
    q.erase(it);
    return tmp;
  }

  void fixCodeVectors(const std::vector<vec> &trainingSet, std::vector<vec> &codeVectors, std::vector<size_t> &assignedCodeVector)
  {
    std::vector<std::vector<size_t>> codeVectorArea(codeVectors.size());


    for(size_t i = 0; i < trainingSet.size(); i++)
    {
      int cur = assignedCodeVector[i];
      codeVectorArea[cur].push_back(i);
    }

    auto cmp = [](const std::vector<size_t> &a, const std::vector<size_t> &b)
    {
      return a.size() < b.size();
    };

    std::set<std::vector<size_t>, decltype(cmp)> q(cmp);

    for(auto &a : codeVectorArea)
      q.emplace(std::move(a));
    codeVectorArea.clear();

    while(q.rbegin()->size() == 0)
    {
      auto mx = extract_max(q);
      auto mn = extract_min(q);

      std::copy(std::begin(mx), std::begin(mx)+mx.size()/2, std::back_inserter(mn));

      mx.erase(mx.begin()+mx.size()/2, mx.end());

      q.emplace(std::move(mx));
      q.emplace(std::move(mn));
    }

    for(auto &a : q)
      codeVectorArea.emplace_back(std::move(a));
    q.clear();

    for(auto &a : codeVectors) for(auto &b : a) b = 0.0;

    for(size_t i = 0; i < codeVectors.size(); i++)
    {
      for(auto &x : codeVectorArea[i])
        codeVectors[i] += trainingSet.at(x);
      codeVectors[i] /= codeVectorArea[i].size();
    }

    assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
  }

}

const int MAX_IT = 50;
std::tuple<std::vector<vec>, std::vector<size_t>, vecType> quantize(const std::vector<vec> &trainingSet, size_t n, vecType eps)
{
  size_t dim = trainingSet[0].size();
  size_t maxCodeVectors = 1 << n;

  std::vector<size_t> assignedCodeVector(trainingSet.size());
  std::vector<vec> codeVectors(1, vec(dim));
  codeVectors.reserve(maxCodeVectors);

  // Initialize values
  for(auto &v : trainingSet) codeVectors[0] += v;
  codeVectors[0] /= (vecType)(trainingSet.size());
  for(auto &a : assignedCodeVector) a = 0;
  vecType distortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);

  while(codeVectors.size() < maxCodeVectors)
  {

    DEBUG_PRINT(eps);
    // splitting phase
    concat(codeVectors, codeVectors);
    for(size_t i = 0; i < codeVectors.size()/2; i++)
    {
      codeVectors[i] *= (vecType)(1+eps);
      codeVectors[i+codeVectors.size()/2] *= (vecType)(1-eps);
    }

    // iteration phase
    for(size_t it = 0; it < MAX_IT; it++)
    {
      assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
      fixCodeVectors(trainingSet, codeVectors, assignedCodeVector);

      vecType newDistortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);
      //if((distortion-newDistortion)/distortion <= eps)
      //{
      //  distortion = newDistortion;
      //  break;
      //}
      distortion = newDistortion;
      //DEBUG_PRINT(it);
      //DEBUG_PRINT(distortion);
    }
  }

  assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
  fixCodeVectors(trainingSet, codeVectors, assignedCodeVector);
  distortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);
  return std::make_tuple(codeVectors, assignedCodeVector, distortion);
}


vecType scale(char x)
{
  return (((vecType)x));
}

char unscale(vecType x)
{
  return char((std::round(x)));
}

std::vector<vec> getBlocksAsVectorsFromImage(const RGBImage& image, int w, int h)
{
  const std::vector<RGB> &img = image.img;

  int xSize = image.xSize;
  int ySize = image.ySize;

  size_t wBlocks = (xSize+w-1)/w;
  size_t hBlocks = (ySize+h-1)/h;

  std::vector<vec> res(wBlocks*hBlocks);

  for(size_t i = 0; i < wBlocks; i++)
    for(size_t j = 0; j < hBlocks; j++)
    {
      vec tmp(3*w*h);
      for(size_t x = i*w; x < i*w+w; x++)
        for(size_t y = j*h; y < j*h+h; y++)
        {
          size_t imgIndex = x*ySize + y;
          size_t vecIndex = ((x-i*w)*h+(y-j*h))*3;
          for(auto shift : RGBRange)
            if(imgIndex < img.size())
              tmp.at(vecIndex+shift) = scale(img.at(imgIndex).at(shift));
            else
              tmp.at(vecIndex+shift) = 0;
        }
      res.at(i*hBlocks+j) = std::move(tmp);
    }
  return res;
}

RGBImage getImageFromVectors(const std::vector<vec> &blocks, int xSize, int ySize, int w, int h)
{
  size_t wBlocks = (xSize+w-1)/w;
  size_t hBlocks = (ySize+h-1)/h;

  std::vector<RGB> imgData(xSize*ySize);

  for(size_t i = 0; i < wBlocks; i++)
    for(size_t j = 0; j < hBlocks; j++)
      {
        const vec &tmp = blocks.at(i*hBlocks+j);

        for(size_t x = i*w; x < i*w+w; x++)
          for(size_t y = j*h; y < j*h+h; y++)
          {
            size_t imgIndex = x*ySize + y;
            size_t vecIndex = ((x-i*w)*h+(y-j*h))*3;
            if(imgIndex < imgData.size())
              for(auto shift : RGBRange)
                imgData.at(imgIndex).at(shift) = unscale(tmp.at(vecIndex+shift));
          }
      }

  RGBImage img;
  img.ySize = ySize;
  img.xSize = xSize;
  img.img = std::move(imgData);
  return img;
}

std::chrono::duration<double> measureExecutionTime(const std::function<void()> &f)
{
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  f();

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> executionTime = end-start;

  return executionTime;
}

std::pair<CompressedImage, CompressionRaport> compress(const RGBImage& image, int blockWidth, int blockHeight, vecType eps, int N)
{
  std::vector<vec> codeVectors;
  std::vector<size_t> assignedCodeVector;
  vecType distortion;

  auto compressionTime = measureExecutionTime([&](){
      auto trainingSet = getBlocksAsVectorsFromImage(image, blockWidth, blockHeight);
      std::tie(codeVectors, assignedCodeVector, distortion) =
          quantize(trainingSet, N, eps);
    });

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
  RGBImage res = getImageFromVectors(quantizedTrainingSet, cImg.xSize, cImg.ySize, cImg.blockWidth, cImg.blockHeight);
  return res;
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
  throw std::runtime_error("not implemented");
}

void CompressedImage::loadFromFile(const std::string &path)
{
  throw std::runtime_error("not implemented");
}

std::ostream& operator <<(std::ostream& stream, const CompressionRaport& raport)
{
  stream << "Compression raport: " << std::endl;
  stream << "distortion       = " << raport.distortion << std::endl;
  stream << "bits per pixel   = " << raport.bitsPerPixel << std::endl;
  stream << "compression time = " << raport.compressionTime.count() << "s" << std::endl;
  return stream;
}
