#pragma once
#include <vector>
#include <algorithm>
#include <numeric>

template<typename T>
std::vector<T> operator+(const std::vector<T> &lhs, const std::vector<T> &rhs)
{
  std::vector<T> res;
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
                 [](const T &a, const T &b){ return a+b; });
  return res;
}

template<typename T>
std::vector<T> operator+=(std::vector<T> &lhs, const std::vector<T> &rhs)
{
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
                 [](const T &a, const T &b){ return a+b; });
  return lhs;
}

template<typename T>
std::vector<T> operator-(const std::vector<T> &lhs, const std::vector<T> &rhs)
{
  std::vector<T> res;
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
                 [](const T &a, const T &b){ return a-b; });
  return res;
}

template<typename T>
std::vector<T> operator-=(std::vector<T> &lhs, const std::vector<T> &rhs)
{
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
                 [](const T &a, const T &b){ return a-b; });
  return lhs;
}

template<typename T>
std::vector<T> operator*(const std::vector<T> &lhs, const std::vector<T> &rhs)
{
  std::vector<T> res;
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
                 [](const T &a, const T &b){ return a*b; });
  return res;
}

template<typename T>
std::vector<T> operator*=(std::vector<T> &lhs, const std::vector<T> &rhs)
{
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
                 [](const T &a, const T &b){ return a*b; });
  return lhs;
}

template<typename T>
std::vector<T> operator/(const std::vector<T> &lhs, const std::vector<T> &rhs)
{
  std::vector<T> res;
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), std::back_inserter(res),
                 [](const T &a, const T &b){ return a/b; });
  return res;
}

template<typename T>
std::vector<T> operator/=(std::vector<T> &lhs, const std::vector<T> &rhs)
{
  std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(),
                 [](const T &a, const T &b){ return a/b; });
  return lhs;
}

template<typename T>
std::vector<T> operator*(const std::vector<T> &lhs, const T& c)
{
  std::vector<T> res;
  std::transform(lhs.begin(), lhs.end(), std::back_inserter(res),
                 [&](const T &a){ return a*c; });
  return res;
}

template<typename T>
std::vector<T> operator*=(std::vector<T> &lhs, const T& c)
{
  std::transform(lhs.begin(), lhs.end(), lhs.begin(),
                 [&](const T &a){ return a*c; });
  return lhs;
}

template<typename T>
std::vector<T> operator/(const std::vector<T> &lhs, const T& c)
{
  std::vector<T> res;
  std::transform(lhs.begin(), lhs.end(), std::back_inserter(res),
                 [&](const T &a){ return a/c; });
  return res;
}

template<typename T>
std::vector<T> operator/=(std::vector<T> &lhs, const T& c)
{
  std::transform(lhs.begin(), lhs.end(), lhs.begin(),
                 [&](const T &a){ return a/c; });
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

template<typename T>
T inner_product(const std::vector<T> &v)
{
  return std::inner_product(v.begin(), v.end(), v.begin());
}
