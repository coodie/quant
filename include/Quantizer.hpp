#pragma once
#include "VectorOperations.hpp"

#include <memory>
#include <tuple>

enum class Quantizers
{LBG, MEDIAN_CUT, LBG_MEDIAN_CUT};

class AbstractQuantizer
{
public:
  virtual std::tuple<std::vector<vec>, std::vector<size_t>, vecType> quantize(const std::vector<vec> &trainingSet, size_t n, vecType eps) = 0;
  virtual ~AbstractQuantizer() = default;
};

typedef std::unique_ptr<AbstractQuantizer> QuantizerPtr;

QuantizerPtr getQuantizer(Quantizers);
