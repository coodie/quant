#include "KDTree.hpp"
#include "KDTreeVectorOfVectorsAdaptor.hpp"

const int KD_LEAF_MAX_SIZE = 10;
typedef KDTreeVectorOfVectorsAdaptor< std::vector<Vector>, VectorType > nanoflannKDTree;

class KDTree::KDTreeImpl
{
public:
  KDTreeImpl(size_t dim, const std::vector<Vector>& pts)
    : kdtree(dim, pts)
  {
    kdtree.index->buildIndex();
  }
  nanoflannKDTree kdtree;
};

KDTree::KDTree(size_t dim, const std::vector<Vector>& pts)
{
  impl.reset(new KDTreeImpl(dim, pts));
}

size_t KDTree::nearestNeighbour(const Vector &pt) const
{
  static size_t num_results = 1;
  static std::vector<size_t> ret_indexes(num_results);
  static std::vector<VectorType> out_dists_sqr(num_results);
  static nanoflann::KNNResultSet<VectorType> resultSet(num_results);
  resultSet.init(&ret_indexes[0], &out_dists_sqr[0] );
  impl->kdtree.index->findNeighbors(resultSet, &pt[0], nanoflann::SearchParams(10));
  return ret_indexes[0];
}

KDTree::~KDTree() = default;

