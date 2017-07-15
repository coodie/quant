#include "Quantizer.hpp"
#include "KDTree.hpp"

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
