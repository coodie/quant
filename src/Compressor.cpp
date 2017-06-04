#include "Compressor.hpp"
#include "ProgramParameters.hpp"
#include "Debug.hpp"

#include "KDTreeVectorOfVectorsAdaptor.hpp"

#include <VectorOperations.hpp>

typedef float vecType;
typedef std::vector<vecType> vec;

namespace
{
  std::vector<vec> getVectorizedBlocks(const RGBImage& image, int w, int h)
  {
    const std::vector<char> &img = image.img;

    const int &xSize = image.xSize*3;
    const int &ySize = image.ySize;

    std::vector<vec> res;
    for(int i = 0; i < xSize/w; i++)
      for(int j = 0; j < ySize/h; j++)
      {
        vec tmp;
        tmp.reserve(w*h);

        for(int x = i*w; x < i*w+w; x++)
          for(int y = j*h; y < j*h+h; y++)
          {
            size_t index = x*xSize + y;
            if(index < img.size())
              tmp.push_back((vecType)img[index]);
            else
              tmp.push_back(0.0);
          }
        res.push_back(std::move(tmp));
      }
    return res;
  }
}

typedef KDTreeVectorOfVectorsAdaptor< std::vector<vec>, vecType > vecKDTree;

const int KD_LEAF_MAX_SIZE = 10;
const int MAX_IT = 50;


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

std::pair<std::vector<vec>, std::vector<size_t>> quantize(const std::vector<vec> &trainingSet, size_t n, float eps)
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
  float distortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);

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
    for(size_t it = 0; it < MAX_IT; it++)
    {
      vecKDTree kdtree(dim, codeVectors, KD_LEAF_MAX_SIZE);
      kdtree.index->buildIndex();

      for(size_t i = 0; i < M; i++)
      {
        size_t found = knnSearch(kdtree, trainingSet[i]);
        assignedCodeVector[i] = found;
      }

      std::vector<vec> newCodeVectors = codeVectors;
    }
  }
  return std::make_pair(codeVectors, assignedCodeVector);
}

CompressedImage compress(const RGBImage& image)
{
  ProgramParameters *par = getParams();

  std::vector<vec> trainingSet = getVectorizedBlocks(image, par->width, par->height);

  auto x = quantize(trainingSet, par->n, par->eps);

  return CompressedImage();
}

