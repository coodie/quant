cmake_minimum_required(VERSION 3.5.1)
project (quant)

include_directories(include)
add_compile_options(--std=c++14)

add_executable(quant
  src/main.cpp
  src/RGBImage.cpp)


