#include "RGBImage.hpp"
#include "ProgramParameters.hpp"
#include "Debug.hpp"
#include "Compressor.hpp"
#include "nanoflann.hpp"

#include <iostream>
#include "boost/program_options.hpp"

std::unique_ptr<ProgramParameters> par;

ProgramParameters* getParams()
{
  return par.get();
}

int main(int argc, char **argv)
{
  namespace po = boost::program_options;
  po::options_description desc("Options");
  par = std::make_unique<ProgramParameters>();

  desc.add_options()
    ("help", "Print help messages")
    (",n", po::value<int>(&par->n)->default_value(10),
     "2^n is number of codevectors")

    (",eps", po::value<float>(&par->eps)->default_value(0.01),
     "eps parameter for quantization algorithm")

    (",w", po::value<int>(&par->width)->default_value(2),
     "Width of block")

    (",h", po::value<int>(&par->height)->default_value(2),
     "Height of block")

    (",d", po::value<bool>(&par->d)->default_value(false),
     "Specify whether to compress or decompress")

    ("file", po::value<std::string>(&par->file)->required(),
     "File to compress/decompress")

    ("saveto,o", po::value<std::string>(&par->saveto)->required(),
     "Save to");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if(vm.count("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }
  vm.notify();

  RGBImage img(par->file);
  img.saveToFile(par->saveto);

  CompressedImage cImg = compress(img);
}
