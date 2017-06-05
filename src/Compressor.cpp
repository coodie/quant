#include "Compressor.hpp"
#include "ProgramParameters.hpp"
#include "Debug.hpp"

#include "KDTreeVectorOfVectorsAdaptor.hpp"

#include <VectorOperations.hpp>

typedef double vecType;
typedef std::vector<vecType> vec;

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
}

typedef KDTreeVectorOfVectorsAdaptor< std::vector<vec>, vecType > vecKDTree;

const int KD_LEAF_MAX_SIZE = 10;
const int MAX_IT = 10;


size_t knnSearch(const vecKDTree &kdtree, const vec &pt)
{
  static size_t num_results = 1;
  static std::vector<size_t> ret_indexes(num_results);
  static std::vector<double> out_dists_sqr(num_results);
  static nanoflann::KNNResultSet<double> resultSet(num_results);
  resultSet.init(&ret_indexes[0], &out_dists_sqr[0] );
  kdtree.index->findNeighbors(resultSet, &pt[0], nanoflann::SearchParams(10));
  return ret_indexes[0];
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

std::pair<std::vector<vec>, std::vector<size_t>> quantize(const std::vector<vec> &trainingSet, size_t n, vecType eps)
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
      codeVectors[i] *= (vecType)(1.0+eps);
      codeVectors[i+codeVectors.size()/2] *= (vecType)(1.0-eps);
    }

    // iteration phase
    std::vector<vec> newCodeVectors;
    for(size_t it = 0; it < MAX_IT; it++)
    {
      vecKDTree kdtree(dim, codeVectors, KD_LEAF_MAX_SIZE);
      kdtree.index->buildIndex();

      for(size_t i = 0; i < M; i++)
      {
        size_t found = knnSearch(kdtree, trainingSet[i]);
        assignedCodeVector[i] = found;
      }

      newCodeVectors = std::vector<vec>(codeVectors.size(), vec(dim));
      std::vector<size_t> assignedCounts(codeVectors.size());

      for(size_t i = 0; i < M; i++)
      {
        int cur = assignedCodeVector[i];
        const vec &x = trainingSet[i];

        newCodeVectors[cur] += x;
        assignedCounts[cur] ++;
      }

      for(size_t i = 0; i < newCodeVectors.size(); i++)
        if(assignedCounts[i] != 0)
          newCodeVectors[i] /= (vecType)assignedCounts[i];

      vecType newDistortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);
       if((distortion-newDistortion)/distortion > eps)
         break;
      distortion = newDistortion;
    }
    codeVectors = newCodeVectors;
  }
  return std::make_pair(codeVectors, assignedCodeVector);
}

RGBImage compress(const RGBImage& image)
{
  ProgramParameters *par = getParams();

  auto trainingSet = getBlocksAsVectorsFromImage(image, par->width, par->height);

  std::vector<vec> codeVectors;
  std::vector<size_t> assignedCodeVector;
  std::tie(codeVectors, assignedCodeVector) = quantize(trainingSet, par->n, par->eps);

  std::vector<vec> quantizedTrainingSet(trainingSet.size());

  for(size_t i = 0; i < quantizedTrainingSet.size(); i++)
    quantizedTrainingSet[i] = codeVectors[assignedCodeVector[i]];

  RGBImage quantizedImg = getImageFromVectors(quantizedTrainingSet, image.xSize, image.ySize, par->width, par->height);

  return quantizedImg;
}

