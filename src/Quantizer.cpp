#include "Quantizer.hpp"
#include "KDTree.hpp"
#include <thread>
#include <iostream>

VectorType getDistortion(const std::vector<Vector> &trainingSet,
                         const std::vector<size_t> &assignedCodeVector,
                         const std::vector<Vector> &codeVectors) {
  size_t dim = trainingSet[0].size();
  size_t M = trainingSet.size();

  VectorType res = 0;
  #pragma omp parallel for reduction(+:res)
  for (size_t i = 0; i < trainingSet.size(); i++) {
    const auto &x = trainingSet[i];
    const auto &c = codeVectors[assignedCodeVector[i]];
    res += norm(x - c) / ((VectorType)(M * dim));
  }

  return res;
}

size_t ncpu = std::thread::hardware_concurrency();

void assignCodeVectors(const std::vector<Vector> &trainingSet,
                       const std::vector<Vector> &codeVectors,
                       std::vector<size_t> &assignedCodeVector) {
  size_t dim = trainingSet.front().size();
  const KDTree kdtree(dim, codeVectors);

  #pragma omp parallel for
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

  #pragma omp parallel for reduction(+:res)
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

  #pragma omp parallel for
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
                VectorType eps, const size_t MAX_IT = 100) {
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
      #pragma omp parallel for
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

VectorType genRandom()
{
  thread_local std::random_device rd;
  thread_local std::mt19937 gen(rd());
  thread_local std::uniform_real_distribution<> dis(0, std::nextafter(1.0, std::numeric_limits<VectorType>::max()));
  return dis(gen);
}

size_t genRandomInt(size_t range)
{
  thread_local std::random_device rd;
  thread_local std::mt19937 gen(rd());
  thread_local std::uniform_int_distribution<size_t> dis(0, 1'000'000'000);
  return dis(gen)%range;
}

std::vector<size_t> randomVector(size_t n, size_t range)
{
  std::vector<size_t> res;
  for(size_t i = 0; i < n; i++)
    res.push_back(genRandomInt(range));
  std::sort(std::begin(res), std::end(res));
  res.erase(std::unique(std::begin(res), std::end(res)), std::end(res));

  return res;
}

class ABCQuantizer : public AbstractQuantizer 
{
  /* Set of codevectors, assigned codevector, fitness value, number of times this solutions was better */
  struct Solution
  {
    std::vector<Vector> codeVectors;
    std::vector<size_t> assignedCodeVector;
    size_t attempts;
    VectorType fitness;
  };

public:
  virtual std::tuple<std::vector<Vector>, std::vector<size_t>, VectorType>
  quantize(const std::vector<Vector> &trainingSet, size_t n, VectorType eps) {

    solutionSize = 1 << n;
    dim = trainingSet.front().size();

    lowerBound.resize(dim);
    upperBound.resize(dim);

    for(size_t i = 0; i < dim; i++)
    {
      auto cmp = [i](const Vector &a, const Vector &b)
      {
        return a[i] < b[i];
      };
      lowerBound[i] = std::min_element(begin(trainingSet), end(trainingSet), cmp)->at(i);
      upperBound[i] = std::max_element(begin(trainingSet), end(trainingSet), cmp)->at(i);
    }

    size_t noRandomDims = std::max(1.0f, randomDimsRatio * solutionSize);

    std::vector<Solution> solutions(employedBee);

    for(size_t i = 0; i < employedBee; i++) 
      solutions[i] = randomizeSolution(trainingSet);

    for(size_t cycle = 0; cycle < MCN; cycle++)
    {
      std::cout << cycle << std::endl;
      auto singleExploit = [&](size_t i)
      {
        size_t rs = genRandomInt(solutions.size());
        while(rs != i)
          rs = genRandomInt(solutions.size());
        exploit(solutions[i], solutions[rs], 
            randomVector(noRandomDims, solutions.size()), eps, trainingSet);
      };

      for(size_t i = 0; i < solutions.size(); i++)
        singleExploit(i);

      VectorType fitnessSum = 0.0;
      for(auto &a : solutions)
        fitnessSum += a.fitness;

      for(size_t i = 0; i < solutions.size(); i++)
      {
        VectorType r = genRandom();
        if(r < solutions[i].fitness/fitnessSum)
          singleExploit(i);
      }

      for(auto &s : solutions)
        if(s.attempts >= limit)
          s = randomizeSolution(trainingSet);
    }
    auto it = std::max_element(std::begin(solutions), std::end(solutions), 
        [](const auto &a, const auto &b)
        {
          return a.fitness < b.fitness;
        });

    return std::make_tuple(it->codeVectors, it->assignedCodeVector, it->fitness);
  }

private:

  void exploit(Solution &solution,
               const Solution &randomSolution,
               std::vector<size_t> randomDims,
               VectorType eps,
      const std::vector<Vector> &trainingSet)
  {
    VectorType distortion = getDistortion(trainingSet, solution.assignedCodeVector, solution.codeVectors); 
    LBGIterate(trainingSet, solution.assignedCodeVector, solution.codeVectors, distortion, eps, LGBIterations);
    assignCodeVectors(trainingSet, solution.codeVectors, solution.assignedCodeVector);
    distortion = getDistortion(trainingSet, solution.assignedCodeVector, solution.codeVectors);

    solution.fitness = fitness(distortion);
    Solution oldSolution = solution;

    for(auto &curDim : randomDims)
    {
      auto& x = solution.codeVectors[curDim];
      auto& y = randomSolution.codeVectors[curDim];
      auto r = genRandom()*2-1;
      x += (x-y)*r;
    }

    assignCodeVectors(trainingSet, solution.codeVectors, solution.assignedCodeVector);
    VectorType newDistortion = getDistortion(trainingSet, solution.assignedCodeVector, solution.codeVectors); 

    if(newDistortion < distortion)
    {
      solution.attempts = 0;
      solution.fitness = fitness(newDistortion);
    }
    else
    {
      solution = std::move(oldSolution);
      solution.attempts++;
    }
  }

  VectorType fitness(VectorType x)
  {
    return 1/(1+x);
  }

  Solution randomizeSolution(const std::vector<Vector> &trainingSet)
  {
    Solution res;
    initializeSolution(res, trainingSet.size());

    for(size_t i = 0; i < solutionSize; i++)
    {
      for(size_t j = 0; j < dim; j++)
      {
        VectorType r = genRandom();
        res.codeVectors[i][j] = lowerBound[j]+(upperBound[j]-lowerBound[j])*r;
      }
    }
    assignCodeVectors(trainingSet, res.codeVectors, res.assignedCodeVector);
    VectorType distortion = getDistortion(trainingSet, res.assignedCodeVector, res.codeVectors);
    res.fitness = fitness(distortion);
    return res;
  }

  void initializeSolution(Solution& s, size_t trainingSetSize)
  {
    s.codeVectors.resize(solutionSize);
    s.assignedCodeVector.resize(trainingSetSize);

    for(auto &x : s.codeVectors) x.resize(dim);
  }
  
  size_t solutionSize;
  size_t dim;

  std::vector<VectorType> lowerBound;
  std::vector<VectorType> upperBound;

  const size_t employedBee = 20;
  const size_t onlookerBee = employedBee;
  const size_t limit = 5;
  const size_t MCN = 50;
  const size_t LGBIterations = 10;
  const float randomDimsRatio = 0.1;
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
  case Quantizers::ABC:
    return QuantizerPtr(new ABCQuantizer());
    break;
  }
  return nullptr;
}
