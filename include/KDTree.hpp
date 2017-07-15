#pragma once
#include "VectorOperations.hpp"
#include <memory>

// KDTree class which is wrapper on nanoflann KDTree implementation

class KDTree
{
public:
  KDTree(size_t dim, const std::vector<Vector>&);
  size_t nearestNeighbour(const Vector &pt) const;
  ~KDTree();

private:
  class KDTreeImpl;
  std::unique_ptr<KDTreeImpl> impl;
};
