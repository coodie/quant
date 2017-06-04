#pragma once
#include <vector>
#include <iostream>

#define DEBUG_PRINT(x) std::cout << "#x = " << (x) << std::endl

template<typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
  out << "[";
  size_t last = v.size() - 1;
  for(size_t i = 0; i < v.size(); ++i) {
    out << v[i];
    if (i != last) 
      out << ", ";
  }
  out << "]";
  return out;
}
