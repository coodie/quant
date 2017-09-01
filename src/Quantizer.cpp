#include "Quantizer.hpp"
#include "KDTree.hpp"

#include <iostream>

class Solution
{
public:
  VectorType updateDistortion() {
    VectorType res = 0;
    #pragma omp parallel for reduction(+:res)
    for (size_t i = 0; i < trainingSet.size(); i++) {
      const auto &x = trainingSet[i];
      const auto &c = codeVectors[assignedCodeVector[i]];
      res += norm(x - c);
    }

    res /= ((VectorType)(trainingSet.size() * dim));

    distortion = res;
    return res;
  }

  void assignCodeVectors() {
    const KDTree kdtree(dim, codeVectors);
  
    #pragma omp parallel for
    for (size_t i = 0; i < trainingSet.size(); i++) {
      size_t found = kdtree.nearestNeighbour(trainingSet[i]);
      assignedCodeVector[i] = found;
    }
  }

  VectorType getDistortionInArea(const std::vector<size_t> &area,
                                 const Vector &codeVector) {
    VectorType res = 0;
    #pragma omp parallel for reduction(+:res)
    for (size_t i = 0; i < area.size(); i++) {
      const auto &x = trainingSet[area[i]];
      auto tmp = norm(x - codeVector);
      res += tmp;
    }
    return res / ((VectorType)(trainingSet.size() * dim));
  }

  Vector trainingSetSum() {
    Vector sum(dim);
    Vector c(dim);
  
    for (size_t i = 0; i < trainingSet.size(); i++) {
      Vector y = trainingSet[i] - c;
      Vector t = sum + y;
      c = (t - sum) - y;
      sum = t;
    }
    return sum;
  }

  Vector sumInArea(const std::vector<size_t> &area) {
    Vector sum(dim);
    Vector c(dim);
  
    for (auto x : area) {
      Vector y = trainingSet[x] - c;
      Vector t = sum + y;
      c = (t - sum) - y;
      sum = t;
    }
    return sum;
  }

  void fixCodeVectors() {
    std::vector<std::vector<size_t>> codeVectorArea(codeVectors.size());
  
    for (size_t i = 0; i < trainingSet.size(); i++) {
      int cur = assignedCodeVector[i];
      codeVectorArea[cur].push_back(i);
    }

    #pragma omp parallel for
    for (size_t i = 0; i < codeVectors.size(); i++) {
      codeVectors[i] = sumInArea(codeVectorArea[i]);
  
      if (codeVectorArea[i].size())
        codeVectors[i] /= (VectorType)codeVectorArea[i].size();
    }
  }

  Solution(const std::vector<Vector> &trainingSet, size_t codeVectorsSize, VectorType eps) :
    trainingSet(trainingSet),
    dim(trainingSet.at(0).size()),
    eps(eps)
  {
    codeVectors.resize(codeVectorsSize);
    assignedCodeVector.resize(trainingSet.size());
  }

  void LBGIterate(const size_t MAX_IT = 100) {
    assignCodeVectors();
    updateDistortion();
    for (size_t it = 0; it < MAX_IT; it++) {
      fixCodeVectors();
      VectorType oldDistortion = distortion;
      updateDistortion();
      if (std::abs(oldDistortion - distortion) / oldDistortion <= eps)
        break;
    }
  }

public:
  const std::vector<Vector> &trainingSet;
  std::vector<size_t> assignedCodeVector;
  std::vector<Vector> codeVectors;
  VectorType distortion;
  const VectorType dim;
  const VectorType eps;
};

class LBGQuantizer : public AbstractQuantizer {
public:
  virtual std::tuple<std::vector<Vector>, std::vector<size_t>, VectorType>
  quantize(const std::vector<Vector> &trainingSet, size_t bitsPerCodeVector, VectorType eps) {

    Solution solution(trainingSet, 1, eps);
    size_t maxCodeVectors = 1 << bitsPerCodeVector;

    auto &codeVectors = solution.codeVectors;
    // Initialize values
    codeVectors[0] = solution.trainingSetSum();
    codeVectors[0] /= (VectorType)(trainingSet.size());

    while (solution.codeVectors.size() < maxCodeVectors) {
      // splitting phase
      concat(codeVectors, codeVectors);
      for (size_t i = 0; i < codeVectors.size() / 2; i++) {
        codeVectors[i] *= (VectorType)(1 + 0.2);
        codeVectors[i + codeVectors.size() / 2] *= (VectorType)(1 - 0.2);
      }

      solution.LBGIterate();
    }
    return std::make_tuple(codeVectors, solution.assignedCodeVector, solution.distortion);
  }
};

QuantizerPtr getQuantizer(Quantizers q) {
  switch (q) {
  case Quantizers::LBG:
    return QuantizerPtr(new LBGQuantizer());
    break;
  default:
    return nullptr;
  }
  return nullptr;
}
