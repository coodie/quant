#pragma once
#include <iostream>
#include <vector>
#include "boost/container/small_vector.hpp"

#define DEBUG_PRINT(x) std::cout << #x " = " << (x) << std::endl

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v) {
  out << "[";
  size_t last = v.size() - 1;
  for (size_t i = 0; i < v.size(); ++i) {
    out << v[i];
    if (i != last) out << ", ";
  }
  out << "]";
  return out;
}

template <typename T, size_t N>
std::ostream &operator<<(std::ostream &out,
                         const boost::container::small_vector<T, N> &v) {
  out << "[";
  size_t last = v.size() - 1;
  for (size_t i = 0; i < v.size(); ++i) {
    out << v[i];
    if (i != last) out << ", ";
  }
  out << "]";
  return out;
}
