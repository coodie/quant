#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include "boost/container/small_vector.hpp"

typedef float vecType;
typedef boost::container::small_vector<vecType, 16> vec;

static inline vec operator+(const vec &lhs, const vec &rhs)
{
  vec res;
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
                 [](const vecType &a, const vecType &b){ return a+b; });
  return res;
}

static inline vec operator+=(vec &lhs, const vec &rhs)
{
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
                 [](const vecType &a, const vecType &b){ return a+b; });
  return lhs;
}

static inline vec operator-(const vec &lhs, const vec &rhs)
{
  vec res;
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
                 [](const vecType &a, const vecType &b){ return a-b; });
  return res;
}

static inline vec operator-=(vec &lhs, const vec &rhs)
{
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
                 [](const vecType &a, const vecType &b){ return a-b; });
  return lhs;
}

static inline vec operator*(const vec &lhs, const vec &rhs)
{
  vec res;
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
                 [](const vecType &a, const vecType &b){ return a*b; });
  return res;
}

static inline vec operator*=(vec &lhs, const vec &rhs)
{
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
                 [](const vecType &a, const vecType &b){ return a*b; });
  return lhs;
}

static inline vec operator/(const vec &lhs, const vec &rhs)
{
  vec res;
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
                 [](const vecType &a, const vecType &b){ return a/b; });
  return res;
}

static inline vec operator/=(vec &lhs, const vec &rhs)
{
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
                 [](const vecType &a, const vecType &b){ return a/b; });
  return lhs;
}

static inline vec operator*(const vec &lhs, const vecType& c)
{
  vec res;
  std::transform(lhs.begin(), lhs.end(), std::back_inserter(res),
                 [&](const vecType &a){ return a*c; });
  return res;
}

static inline vec operator*=(vec &lhs, const vecType& c)
{
  std::transform(lhs.begin(), lhs.end(), lhs.begin(),
                 [&](const vecType &a){ return a*c; });
  return lhs;
}

static inline vec operator/(const vec &lhs, const vecType& c)
{
  vec res;
  std::transform(lhs.begin(), lhs.end(), std::back_inserter(res),
                 [&](const vecType &a){ return a/c; });
  return res;
}

static inline vec operator/=(vec &lhs, const vecType& c)
{
  std::transform(lhs.begin(), lhs.end(), lhs.begin(),
                 [&](const vecType &a){ return a/c; });
  return lhs;
}

template<typename T>
std::vector<T> concat(std::vector<T> &lhs, const std::vector<T> &rhs)
{
  size_t m = rhs.size();
  for(size_t i = 0; i < m; i++)
    lhs.emplace_back(rhs[i]);
  return lhs;
}


static inline vecType inner_product(const vec &v)
{
  return std::inner_product(v.begin(), v.end(), v.begin(), 0);
}
