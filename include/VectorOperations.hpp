#pragma once
#include <algorithm>
#include <numeric>
#include <vector>

#include "boost/container/small_vector.hpp"

// Vector represents vector that is used in algorithms
// VectorType describes type inside Vector
typedef double VectorType;
typedef boost::container::small_vector<VectorType, 27> Vector;
typedef boost::container::small_vector<char, 27> CharVector;

static inline Vector operator+(const Vector &lhs, const Vector &rhs) {
  Vector res;
  std::transform(
      lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
      [](const VectorType &a, const VectorType &b) { return a + b; });
  return res;
}

static inline Vector operator+=(Vector &lhs, const Vector &rhs) {
  std::transform(
      lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
      [](const VectorType &a, const VectorType &b) { return a + b; });
  return lhs;
}

static inline Vector operator-(const Vector &lhs, const Vector &rhs) {
  Vector res;
  std::transform(
      lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
      [](const VectorType &a, const VectorType &b) { return a - b; });
  return res;
}

static inline Vector operator-=(Vector &lhs, const Vector &rhs) {
  std::transform(
      lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
      [](const VectorType &a, const VectorType &b) { return a - b; });
  return lhs;
}

static inline Vector operator*(const Vector &lhs, const Vector &rhs) {
  Vector res;
  std::transform(
      lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
      [](const VectorType &a, const VectorType &b) { return a * b; });
  return res;
}

static inline Vector operator*=(Vector &lhs, const Vector &rhs) {
  std::transform(
      lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
      [](const VectorType &a, const VectorType &b) { return a * b; });
  return lhs;
}

static inline Vector operator/(const Vector &lhs, const Vector &rhs) {
  Vector res;
  std::transform(
      lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
      [](const VectorType &a, const VectorType &b) { return a / b; });
  return res;
}

static inline Vector operator/=(Vector &lhs, const Vector &rhs) {
  std::transform(
      lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
      [](const VectorType &a, const VectorType &b) { return a / b; });
  return lhs;
}

static inline Vector operator*(const Vector &lhs, const VectorType &c) {
  Vector res;
  std::transform(lhs.begin(), lhs.end(), std::back_inserter(res),
                 [&](const VectorType &a) { return a * c; });
  return res;
}

static inline Vector operator*=(Vector &lhs, const VectorType &c) {
  std::transform(lhs.begin(), lhs.end(), lhs.begin(),
                 [&](const VectorType &a) { return a * c; });
  return lhs;
}

static inline Vector operator/(const Vector &lhs, const VectorType &c) {
  Vector res;
  std::transform(lhs.begin(), lhs.end(), std::back_inserter(res),
                 [&](const VectorType &a) { return a / c; });
  return res;
}

static inline Vector operator/=(Vector &lhs, const VectorType &c) {
  std::transform(lhs.begin(), lhs.end(), lhs.begin(),
                 [&](const VectorType &a) { return a / c; });
  return lhs;
}

template <typename T>
std::vector<T> concat(std::vector<T> &lhs, const std::vector<T> &rhs) {
  size_t m = rhs.size();
  for (size_t i = 0; i < m; i++) lhs.emplace_back(rhs[i]);
  return lhs;
}

static inline VectorType norm(const Vector &v) {
  VectorType res = 0.0;
  for (auto x : v) res += x * x;
  return res;
}
