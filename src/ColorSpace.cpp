#include "ColorSpace.hpp"
#include "VectorOperations.hpp"

RGBDouble ColorSpace::RGBtoColorSpace(const RGB &c) {
  return {(VectorType)c.at(0), (VectorType)c.at(1), (VectorType)c.at(2)};
}

RGB ColorSpace::colorSpaceToRGB(const RGBDouble &c) {
  return {(char)std::round(c.at(0)), (char)std::round(c.at(1)),
          (char)std::round(c.at(2))};
}

class ScaledColor : public ColorSpace {
 public:
  ScaledColor() = default;
  RGBDouble RGBtoColorSpace(const RGB &c) override {
    RGBDouble res;
    for (size_t i = 0; i < c.size(); i++)
      res[i] = ((VectorType)c[i] + 128.0) / 255;
    return res;
  }

  RGB colorSpaceToRGB(const RGBDouble &c) override {
    RGB res;
    for (size_t i = 0; i < c.size(); i++)
      res[i] = (char)std::round((c[i] - 128.0) * 255);
    return res;
  }
};

class Cie1931 : public ColorSpace {
 public:
  Cie1931() = default;
  RGBDouble RGBtoColorSpace(const RGB &c) override {
    return {(c[0] * 0.490 + c[1] * 0.310 + c[2] * 0.200) / 0.17697,
            (c[0] * 0.17697 + c[1] * 0.81240 + c[2] * 0.01063) / 0.17697,
            (c[0] * 0 + c[1] * 0.01 + c[2] * 0.99) / 0.17697};
  }

  RGB colorSpaceToRGB(const RGBDouble &c) override {
    RGBDouble tmp = {(c[0] * 0.418 + c[1] * (-0.15866) + c[2] * (-0.082835)),
                     (c[0] * (-0.091169) + c[1] * 0.25243 + c[2] * 0.015708),
                     (c[0] * 0.0009209 + c[1] * (-0.0025498) + c[2] * 0.17860)};
    return {(char)std::round(tmp[0]), (char)std::round(tmp[1]),
            (char)std::round(tmp[2])};
  }
};

ColorSpacePtr getColorSpace(ColorSpaces cs) {
  switch (cs) {
    case ColorSpaces::NORMAL:
      return ColorSpacePtr(new ColorSpace());
      break;
    case ColorSpaces::CIE1931:
      return ColorSpacePtr(new Cie1931());
      break;
    case ColorSpaces::SCALED:
      return ColorSpacePtr(new ScaledColor());
      break;
  }
  return nullptr;
}
