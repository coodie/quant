#include "Compressor.hpp"
#include "Debug.hpp"
#include "KDTree.hpp"
#include "VectorOperations.hpp"

#include <set>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>

RGBDouble ColorSpace::RGBtoColorSpace(const RGB &c)
{
  return {(vecType)c.at(0), (vecType)c.at(1), (vecType)c.at(2)};
}

RGB ColorSpace::colorSpaceToRGB(const RGBDouble &c)
{
  return {(char)std::round(c.at(0)), (char)std::round(c.at(1)), (char)std::round(c.at(2))};
}

class ScaledColor : public ColorSpace
{
public:
  ScaledColor() = default;
  RGBDouble RGBtoColorSpace(const RGB &c) override
  {
    RGBDouble res;
    for(size_t i = 0; i < c.size(); i++)
      res[i] = ((vecType)c[i]+128.0)/255;
    return res;
  }

  RGB colorSpaceToRGB(const RGBDouble &c) override
  {
    RGB res;
    for(size_t i = 0; i < c.size(); i++)
      res[i] = (char)std::round((c[i]-128.0)*255);
    return res;
  }
};


class Cie1931 : public ColorSpace
{
public:
  Cie1931() = default;
  RGBDouble RGBtoColorSpace(const RGB &c) override
  {
    return
      {
        (c[0]*0.490+c[1]*0.310+c[2]*0.200)/0.17697,
          (c[0]*0.17697+c[1]*0.81240+c[2]*0.01063)/0.17697,
          (c[0]*0+c[1]*0.01+c[2]*0.99)/0.17697
          };
  }

  RGB colorSpaceToRGB(const RGBDouble &c) override
  {
    RGBDouble tmp =
      {
        (c[0]*0.418+c[1]*(-0.15866)+c[2]*(-0.082835)),
        (c[0]*(-0.091169)+c[1]*0.25243+c[2]*0.015708),
        (c[0]*0.0009209+c[1]*(-0.0025498)+c[2]*0.17860)
      };
    return {(char)std::round(tmp[0]), (char)std::round(tmp[1]), (char)std::round(tmp[2])};
  }
};


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
    res += norm(x-c)/((vecType)(M*dim));
  }

  return res;
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

class MedianCut : public AbstractQuantizer
{
private:
  size_t findWidestDimension(const std::vector<vec> &vectors)
  {
    std::vector<int> dim(vectors[0].size());
    std::iota(begin(dim), end(dim), 0);

    std::vector<vecType> ranges;

    std::transform(begin(dim), end(dim), std::back_inserter(ranges),
                   [&](auto d)
                   {
                     auto minmax = std::minmax_element(begin(vectors), end(vectors),
                                               [=](const auto &a, const auto &b)
                                               {
                                                 return a[d] < b[d];
                                               });
                     auto &fst = *minmax.first;
                     auto &snd = *minmax.second;
                     auto res = snd[d]-fst[d];
                     return res;
                   });
    auto it = std::max_element(begin(ranges), end(ranges));
    return std::distance(begin(ranges), it);
  }

  std::pair<std::vector<vec>, std::vector<vec>> split(std::vector<vec> vectors, size_t dim)
  {
    std::sort(begin(vectors), end(vectors),
              [=](const auto &a, const auto &b)
              {
                return a[dim] < b[dim];
              });
    auto half = begin(vectors) + vectors.size()/2;
    return std::make_pair(std::vector<vec>(begin(vectors), half),
                          std::vector<vec>(half, end(vectors)));
  }

  std::vector<vec> run(const std::vector<vec> &vectors, size_t depth, size_t max_depth)
  {
    if(depth >= max_depth || vectors.size() <= 1)
    {
      auto avg = std::accumulate(begin(vectors), end(vectors), vec(vectors[0].size()),
                                 [](const auto &a, const auto &b)
                                 {
                                   return a+b;
                                 });

      avg /= (vecType)vectors[0].size();
      return {avg};
    }

    size_t dim = findWidestDimension(vectors);
    auto tmp = split(vectors, dim);
    auto res1 = run(tmp.first, depth+1, max_depth);
    auto res2 = run(tmp.second, depth+1, max_depth);
    concat(res1, res2);
    return res1;
  }

public:
  MedianCut() = default;

  virtual std::tuple<std::vector<vec>, std::vector<size_t>, vecType> quantize(const std::vector<vec> &trainingSet, size_t n, vecType)
  {
    auto codeVectors = run(trainingSet, 0, n);
    std::vector<size_t> assignedCodeVector(trainingSet.size());
    assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
    auto distortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);
    return std::make_tuple(codeVectors, assignedCodeVector, distortion);
  }

};

vecType getDistortionInArea(const std::vector<vec> &trainingSet,
                            const std::vector<size_t> &area,
                            const vec &codeVector)
{
  size_t dim = trainingSet[0].size();
  size_t M = trainingSet.size();

  vecType res = 0;
  for(size_t i = 0; i < area.size(); i++)
  {
    const auto &x = trainingSet[area[i]];
    auto tmp = norm(x-codeVector);
    res += tmp;
  }
  return res/((vecType)(M*dim));
}

vec kahanSum(const std::vector<vec> &trainingSet, const std::vector<size_t> &area)
{
  size_t dim = trainingSet[0].size();
  vec sum(dim);
  vec c(dim);

  for(auto x : area)
  {
    vec y = trainingSet[x] - c;
    vec t = sum + y;
    c = (t - sum) - y;
    sum = t;
  }
  return sum;
}

vec kahanSum(const std::vector<vec> &trainingSet)
{
  size_t dim = trainingSet[0].size();
  vec sum(dim);
  vec c(dim);

  for(auto x : trainingSet)
    {
      vec y = x - c;
      vec t = sum + y;
      c = (t - sum) - y;
      sum = t;
    }
  return sum;
}

void fixCodeVectors(const std::vector<vec> &trainingSet, std::vector<vec> &codeVectors, std::vector<size_t> &assignedCodeVector)
{
  std::vector<std::vector<size_t>> codeVectorArea(codeVectors.size());

  for(size_t i = 0; i < trainingSet.size(); i++)
  {
    int cur = assignedCodeVector[i];
    codeVectorArea[cur].push_back(i);
  }

  auto cmpByDistortion = [&](const size_t &a, const size_t &b)
  {
    vecType distA = getDistortionInArea(trainingSet, codeVectorArea[a], codeVectors[a]);
    vecType distB = getDistortionInArea(trainingSet, codeVectorArea[b], codeVectors[b]);
    if( distA < distB)
      return true;
    return false;
  };

  size_t mx = 0;

  for(size_t i = 0; i < codeVectors.size(); i++)
  {
    if(cmpByDistortion(mx, i))
      mx = i;
  }

  for(size_t i = 0; i < codeVectors.size(); i++)
  {
    codeVectors[i] = kahanSum(trainingSet, codeVectorArea[i]);

    if(codeVectorArea[i].size())
      codeVectors[i] /= (vecType)codeVectorArea[i].size();
    else
      codeVectors[i] = trainingSet[codeVectorArea[mx][std::rand() % codeVectorArea[mx].size()]];
  }

  assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
}

void LBGIterate(const std::vector<vec> &trainingSet, std::vector<size_t> &assignedCodeVector, std::vector<vec> &codeVectors, vecType distortion, vecType eps)
{
  const size_t MAX_IT = 100;
  // iteration phase
  for(size_t it = 0; it < MAX_IT; it++)
    {
      assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
      fixCodeVectors(trainingSet, codeVectors, assignedCodeVector);

      vecType newDistortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);
      if((distortion-newDistortion)/distortion <= eps)
        {
          distortion = newDistortion;
          break;
        }
      distortion = newDistortion;
    }
}

class LBGQuantizer : public AbstractQuantizer
{
public:
  virtual std::tuple<std::vector<vec>, std::vector<size_t>, vecType> quantize(const std::vector<vec> &trainingSet, size_t n, vecType eps)
  {
    size_t dim = trainingSet[0].size();
    size_t maxCodeVectors = 1 << n;

    std::vector<size_t> assignedCodeVector(trainingSet.size());
    std::vector<vec> codeVectors(1, vec(dim));
    codeVectors.reserve(maxCodeVectors);

    // Initialize values
    codeVectors[0] = kahanSum(trainingSet);
    codeVectors[0] /= (vecType)(trainingSet.size());


    for(auto &a : assignedCodeVector) a = 0;
    vecType distortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);

    while(codeVectors.size() < maxCodeVectors)
    {

      // splitting phase
      concat(codeVectors, codeVectors);
      for(size_t i = 0; i < codeVectors.size()/2; i++)
      {
        codeVectors[i] *= (vecType)(1+0.2);
        codeVectors[i+codeVectors.size()/2] *= (vecType)(1-0.2);
      }

      LBGIterate(trainingSet, assignedCodeVector, codeVectors, distortion, eps);
    }

    assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
    fixCodeVectors(trainingSet, codeVectors, assignedCodeVector);
    distortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);
    return std::make_tuple(codeVectors, assignedCodeVector, distortion);
  }
};

class LBGMedianCutQuantizer : public AbstractQuantizer
{

public:
  virtual std::tuple<std::vector<vec>, std::vector<size_t>, vecType> quantize(const std::vector<vec> &trainingSet, size_t n, vecType eps)
  {

    std::vector<size_t> assignedCodeVector;
    std::vector<vec> codeVectors;
    vecType distortion = 0.0;

    std::tie(codeVectors, assignedCodeVector, distortion) = MedianCut().quantize(trainingSet, n, eps);

    LBGIterate(trainingSet, assignedCodeVector, codeVectors, distortion, eps);

    assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
    fixCodeVectors(trainingSet, codeVectors, assignedCodeVector);
    distortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);
    return std::make_tuple(codeVectors, assignedCodeVector, distortion);
  }
};

std::vector<vec> getBlocksAsVectorsFromImage(const RGBImage& image, int w, int h, const ColorSpacePtr &cs)
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
            {
              RGBDouble scaled = cs->RGBtoColorSpace(img.at(imgIndex));
              tmp.at(vecIndex+shift) = scaled.at(shift);
            }
            else
              tmp.at(vecIndex+shift) = 0;
        }
      res.at(i*hBlocks+j) = std::move(tmp);
    }
  return res;
}

RGBImage getImageFromVectors(const std::vector<vec> &blocks, int xSize, int ySize, int w, int h, const ColorSpacePtr &cs)
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
            {
              RGB unscaled = cs->colorSpaceToRGB({tmp.at(vecIndex), tmp.at(vecIndex+1), tmp.at(vecIndex+2)});
              for(auto shift : RGBRange)
                imgData.at(imgIndex).at(shift) = unscaled.at(shift);
            }
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

std::pair<CompressedImage, CompressionRaport> compress(const RGBImage& image,
                                                       const QuantizerPtr &quantizer,
                                                       const ColorSpacePtr &colorSpace,
                                                       int blockWidth,
                                                       int blockHeight,
                                                       vecType eps,
                                                       int N)
{
  std::vector<vec> codeVectors;
  std::vector<size_t> assignedCodeVector;
  vecType distortion;

  auto compressionTime = measureExecutionTime([&](){
      auto trainingSet = getBlocksAsVectorsFromImage(image, blockWidth, blockHeight, colorSpace);
      std::tie(codeVectors, assignedCodeVector, distortion) =
          quantizer->quantize(trainingSet, N, eps);
    });

  CompressedImage resImg;
  resImg.codeVectors = std::move(codeVectors);
  resImg.assignedCodeVector = std::move(assignedCodeVector);
  resImg.xSize = image.xSize;
  resImg.ySize = image.ySize;
  resImg.blockWidth = blockWidth;
  resImg.blockHeight = blockHeight;

  float bitsPerPixel = ((float)resImg.sizeInBits())/(image.xSize*image.ySize);

  // Decompress image temporarily to compute real distortion
  {
    RGBImage img = decompress(resImg, colorSpace);
    distortion = 0;
    for(size_t i = 0; i < image.img.size(); i++)
    {
      for(auto j : RGBRange)
        distortion += std::pow((vecType)image.img[i][j] - (vecType)img.img[i][j], 2);
    }
    distortion /= image.img.size()*3;
  }


  size_t uncompressedSize = image.sizeInBytes();
  size_t compressedSize = resImg.sizeInBits()/8;

  CompressionRaport raport{distortion, bitsPerPixel, uncompressedSize, compressedSize, compressionTime};
  return std::make_pair(resImg, raport);
}

RGBImage decompress(const CompressedImage& cImg, const ColorSpacePtr& cs)
{
  std::vector<vec> quantizedTrainingSet(cImg.assignedCodeVector.size());
  for(size_t i = 0; i < quantizedTrainingSet.size(); i++)
    quantizedTrainingSet[i] = cImg.codeVectors[cImg.assignedCodeVector[i]];

  RGBImage res = getImageFromVectors(quantizedTrainingSet, cImg.xSize, cImg.ySize, cImg.blockWidth, cImg.blockHeight, cs);
  return res;
}

size_t smallestPow2(size_t n)
{
  int p = 0;
  while(n/=2) p++;
  return p;
}

size_t CompressedImage::sizeInBits()
{
  // At this moment approximate size
  size_t codeVectorBits = smallestPow2(codeVectors.size());
  size_t dimension = blockWidth*blockHeight;
  size_t bits = codeVectorBits*assignedCodeVector.size() +
    dimension*codeVectors.size()*8*3; // Store codevectors

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

ColorSpacePtr getColorSpace(ColorSpaces cs)
{
  switch(cs)
  {
  case ColorSpaces::NORMAL:
    return ColorSpacePtr(new ColorSpace());
    break;
  case ColorSpaces::CIE1931:
    return ColorSpacePtr(new Cie1931());
    break;
  case ColorSpaces::SCALED:
    return ColorSpacePtr(new ScaledColor());
    break;
  }
  return nullptr;
}

QuantizerPtr getQuantizer(Quantizers q)
{
switch (q) {
 case Quantizers::LBG:
   return QuantizerPtr(new LBGQuantizer());
   break;
 case Quantizers::MEDIAN_CUT:
   return QuantizerPtr(new MedianCut());
   break;
 case Quantizers::LBG_MEDIAN_CUT:
   return QuantizerPtr(new LBGMedianCutQuantizer());
   break;
}
 return nullptr;
}

std::string prettyPrintBytes(size_t bytes)
{
  std::stringstream res;
  if(bytes < 1024)
  {
    res << bytes << "b";
    return res.str();
  }

  if(bytes < 1024*1024)
  {
    size_t kb = bytes/1024;
    res << kb << "," << bytes%1024 << "Kb";
    return res.str();
  }

  size_t mb = bytes/(1024*1024);
  size_t kb = bytes % (1024*1024);

  res << mb << "," << kb << "Mb";
  return res.str();
}

std::ostream& operator <<(std::ostream& stream, const CompressionRaport& raport)
{
  stream << "Compression raport: " << std::endl;
  stream << "Distortion        = " << std::fixed << std::setprecision(10) << raport.distortion << std::endl;
  stream << "Bits per pixel    = " << raport.bitsPerPixel << std::endl;
  stream << "Uncompressed size = " << prettyPrintBytes(raport.uncompressedSize) << std::endl;
  stream << "Compressed size   = " << prettyPrintBytes(raport.compressedSize) << std::endl;
  stream << "Compression ratio = " << std::fixed <<  std::setprecision(3) << (double)raport.compressedSize/raport.uncompressedSize << std::endl;
  stream << "Compression time  = " << raport.compressionTime.count() << "s" << std::endl;
  return stream;
}
