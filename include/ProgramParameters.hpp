#pragma once
#include <string>
#include <memory>

struct ProgramParameters
{
  int n;
  int width;
  int height;
  float eps;
  bool d;
  bool raport;
  int quantizer;
  std::string file;
  std::string saveto;
};

ProgramParameters* getParams();
void paramsInitialize();
