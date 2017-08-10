#pragma once

#include <memory>
#include "RGBImage.hpp"

enum class ColorSpaces { NORMAL, SCALED, CIE1931 };

class ColorSpace {
 public:
  virtual RGBDouble RGBtoColorSpace(const RGB &);
  virtual RGB colorSpaceToRGB(const RGBDouble &);
  virtual ~ColorSpace() = default;
};

typedef std::unique_ptr<ColorSpace> ColorSpacePtr;

ColorSpacePtr getColorSpace(ColorSpaces);
