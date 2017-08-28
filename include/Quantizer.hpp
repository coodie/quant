#pragma once
#include "VectorOperations.hpp"

#include <memory>
#include <tuple>
#include <thread>

enum class Quantizers { LBG, MEDIAN_CUT, LBG_MEDIAN_CUT };

class AbstractQuantizer {
 public:
  virtual std::tuple<std::vector<Vector>, std::vector<size_t>, VectorType>
  quantize(const std::vector<Vector> &trainingSet, size_t n,
           VectorType eps) = 0;
  virtual ~AbstractQuantizer() = default;
};

typedef std::unique_ptr<AbstractQuantizer> QuantizerPtr;

QuantizerPtr getQuantizer(Quantizers);
