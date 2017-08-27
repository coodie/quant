#include "Quantizer.hpp"
#include "KDTree.hpp"

VectorType getDistortion(const std::vector<Vector> &trainingSet,
                         const std::vector<size_t> &assignedCodeVector,
                         const std::vector<Vector> &codeVectors) {
  size_t dim = trainingSet[0].size();
  size_t M = trainingSet.size();

  VectorType res = 0;
  for (size_t i = 0; i < trainingSet.size(); i++) {
    const auto &x = trainingSet[i];
    const auto &c = codeVectors[assignedCodeVector[i]];
    res += norm(x - c) / ((VectorType)(M * dim));
  }

  return res;
}

void assignCodeVectors(const std::vector<Vector> &trainingSet,
                       const std::vector<Vector> &codeVectors,
                       std::vector<size_t> &assignedCodeVector) {
  size_t dim = trainingSet.front().size();
  KDTree kdtree(dim, codeVectors);

  for (size_t i = 0; i < trainingSet.size(); i++) {
    size_t found = kdtree.nearestNeighbour(trainingSet[i]);
    assignedCodeVector[i] = found;
  }
}

class MedianCut : public AbstractQuantizer {
private:
  size_t findWidestDimension(const std::vector<Vector> &vectors) {
    std::vector<int> dim(vectors[0].size());
    std::iota(begin(dim), end(dim), 0);

    std::vector<VectorType> ranges;

    std::transform(
        begin(dim), end(dim), std::back_inserter(ranges), [&](auto d) {
          auto minmax = std::minmax_element(
              begin(vectors), end(vectors),
              [=](const auto &a, const auto &b) { return a[d] < b[d]; });
          auto &fst = *minmax.first;
          auto &snd = *minmax.second;
          auto res = snd[d] - fst[d];
          return res;
        });
    auto it = std::max_element(begin(ranges), end(ranges));
    return std::distance(begin(ranges), it);
  }

  std::pair<std::vector<Vector>, std::vector<Vector>>
  split(std::vector<Vector> vectors, size_t dim) {
    std::sort(begin(vectors), end(vectors),
              [=](const auto &a, const auto &b) { return a[dim] < b[dim]; });
    auto half = begin(vectors) + vectors.size() / 2;
    return std::make_pair(std::vector<Vector>(begin(vectors), half),
                          std::vector<Vector>(half, end(vectors)));
  }

  std::vector<Vector> run(const std::vector<Vector> &vectors, size_t depth,
                          size_t max_depth) {
    if (depth >= max_depth || vectors.size() <= 1) {
      auto avg = std::accumulate(
          begin(vectors), end(vectors), Vector(vectors[0].size()),
          [](const auto &a, const auto &b) { return a + b; });

      avg /= (VectorType)vectors[0].size();
      return {avg};
    }

    size_t dim = findWidestDimension(vectors);
    auto tmp = split(vectors, dim);
    auto res1 = run(tmp.first, depth + 1, max_depth);
    auto res2 = run(tmp.second, depth + 1, max_depth);
    concat(res1, res2);
    return res1;
  }

public:
  MedianCut() = default;

  virtual std::tuple<std::vector<Vector>, std::vector<size_t>, VectorType>
  quantize(const std::vector<Vector> &trainingSet, size_t n, VectorType) {
    auto codeVectors = run(trainingSet, 0, n);
    std::vector<size_t> assignedCodeVector(trainingSet.size());
    assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
    auto distortion =
        getDistortion(trainingSet, assignedCodeVector, codeVectors);
    return std::make_tuple(codeVectors, assignedCodeVector, distortion);
  }
};

VectorType getDistortionInArea(const std::vector<Vector> &trainingSet,
                               const std::vector<size_t> &area,
                               const Vector &codeVector) {
  size_t dim = trainingSet[0].size();
  size_t M = trainingSet.size();

  VectorType res = 0;
  for (size_t i = 0; i < area.size(); i++) {
    const auto &x = trainingSet[area[i]];
    auto tmp = norm(x - codeVector);
    res += tmp;
  }
  return res / ((VectorType)(M * dim));
}

Vector kahanSum(const std::vector<Vector> &trainingSet,
                const std::vector<size_t> &area) {
  size_t dim = trainingSet[0].size();
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

Vector kahanSum(const std::vector<Vector> &trainingSet) {
  size_t dim = trainingSet[0].size();
  Vector sum(dim);
  Vector c(dim);

  for (auto x : trainingSet) {
    Vector y = x - c;
    Vector t = sum + y;
    c = (t - sum) - y;
    sum = t;
  }
  return sum;
}

void fixCodeVectors(const std::vector<Vector> &trainingSet,
                    std::vector<Vector> &codeVectors,
                    std::vector<size_t> &assignedCodeVector) {
  std::vector<std::vector<size_t>> codeVectorArea(codeVectors.size());

  for (size_t i = 0; i < trainingSet.size(); i++) {
    int cur = assignedCodeVector[i];
    codeVectorArea[cur].push_back(i);
  }

  auto cmpByDistortion = [&](const size_t &a, const size_t &b) {
    VectorType distA =
        getDistortionInArea(trainingSet, codeVectorArea[a], codeVectors[a]);
    VectorType distB =
        getDistortionInArea(trainingSet, codeVectorArea[b], codeVectors[b]);
    if (distA < distB)
      return true;
    return false;
  };

  size_t mx = 0;

  for (size_t i = 0; i < codeVectors.size(); i++) {
    if (cmpByDistortion(mx, i))
      mx = i;
  }

  for (size_t i = 0; i < codeVectors.size(); i++) {
    codeVectors[i] = kahanSum(trainingSet, codeVectorArea[i]);

    if (codeVectorArea[i].size())
      codeVectors[i] /= (VectorType)codeVectorArea[i].size();
    else
      codeVectors[i] =
          trainingSet[codeVectorArea[mx]
                                    [std::rand() % codeVectorArea[mx].size()]];
  }

  assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
}

void LBGIterate(const std::vector<Vector> &trainingSet,
                std::vector<size_t> &assignedCodeVector,
                std::vector<Vector> &codeVectors, VectorType distortion,
                VectorType eps) {
  const size_t MAX_IT = 100;
  // iteration phase
  for (size_t it = 0; it < MAX_IT; it++) {
    assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
    fixCodeVectors(trainingSet, codeVectors, assignedCodeVector);

    VectorType newDistortion =
        getDistortion(trainingSet, assignedCodeVector, codeVectors);
    if ((distortion - newDistortion) / distortion <= eps) {
      distortion = newDistortion;
      break;
    }
    distortion = newDistortion;
  }
}

class LBGQuantizer : public AbstractQuantizer {
public:
  virtual std::tuple<std::vector<Vector>, std::vector<size_t>, VectorType>
  quantize(const std::vector<Vector> &trainingSet, size_t n, VectorType eps) {
    size_t dim = trainingSet[0].size();
    size_t maxCodeVectors = 1 << n;

    std::vector<size_t> assignedCodeVector(trainingSet.size());
    std::vector<Vector> codeVectors(1, Vector(dim));
    codeVectors.reserve(maxCodeVectors);

    // Initialize values
    codeVectors[0] = kahanSum(trainingSet);
    codeVectors[0] /= (VectorType)(trainingSet.size());

    for (auto &a : assignedCodeVector)
      a = 0;
    VectorType distortion =
        getDistortion(trainingSet, assignedCodeVector, codeVectors);

    while (codeVectors.size() < maxCodeVectors) {
      // splitting phase
      concat(codeVectors, codeVectors);
      for (size_t i = 0; i < codeVectors.size() / 2; i++) {
        codeVectors[i] *= (VectorType)(1 + 0.2);
        codeVectors[i + codeVectors.size() / 2] *= (VectorType)(1 - 0.2);
      }

      LBGIterate(trainingSet, assignedCodeVector, codeVectors, distortion, eps);
    }

    assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
    fixCodeVectors(trainingSet, codeVectors, assignedCodeVector);
    distortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);
    return std::make_tuple(codeVectors, assignedCodeVector, distortion);
  }
};

class LBGMedianCutQuantizer : public AbstractQuantizer {
public:
  virtual std::tuple<std::vector<Vector>, std::vector<size_t>, VectorType>
  quantize(const std::vector<Vector> &trainingSet, size_t n, VectorType eps) {
    std::vector<size_t> assignedCodeVector;
    std::vector<Vector> codeVectors;
    VectorType distortion = 0.0;

    std::tie(codeVectors, assignedCodeVector, distortion) =
        MedianCut().quantize(trainingSet, n, eps);

    LBGIterate(trainingSet, assignedCodeVector, codeVectors, distortion, eps);

    assignCodeVectors(trainingSet, codeVectors, assignedCodeVector);
    fixCodeVectors(trainingSet, codeVectors, assignedCodeVector);
    distortion = getDistortion(trainingSet, assignedCodeVector, codeVectors);
    return std::make_tuple(codeVectors, assignedCodeVector, distortion);
  }
};

QuantizerPtr getQuantizer(Quantizers q) {
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
