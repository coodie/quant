#pragma once
#include <memory>
#include <string>

struct ProgramParameters {
  int n;
  int width;
  int height;
  float eps;
  bool d;
  bool raport;
  int quantizer;
  int colorspace;
  std::string file;
  std::string saveto;
};

ProgramParameters *getParams();
void paramsInitialize();
