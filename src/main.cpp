#include "RGBImage.hpp"
#include "ProgramParameters.hpp"
#include "Debug.hpp"
#include "Compressor.hpp"
#include "nanoflann.hpp"

#include <iostream>
#include "boost/program_options.hpp"

int main(int argc, char **argv)
{
  namespace po = boost::program_options;
  po::options_description desc("Options");
  auto par = getParams();
  desc.add_options()
    ("help", "Print help messages")
    (",n", po::value<int>(&par->n)->default_value(8),
     "2^n is number of codevectors")

    (",e", po::value<float>(&par->eps)->default_value(0.01),
     "eps parameter for quantization algorithm")

    (",w", po::value<int>(&par->width)->default_value(3),
     "Width of block")

    (",h", po::value<int>(&par->height)->default_value(3),
     "Height of block")

    ("file", po::value<std::string>(&par->file)->required(),
     "File to compress/decompress")

    ("saveto,o", po::value<std::string>(&par->saveto)->required(),
     "Save to")
    (",r", po::value<bool>(&par->raport)->default_value(true),
     "Print raport to std::out")
    (",quantizer", po::value<int>(&par->quantizer)->default_value((int)Quantizers::LGB),
    "Pick quantizer");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if(vm.count("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }
  vm.notify();

  RGBImage img(par->file);
  CompressedImage cImg;
  CompressionRaport raport;

  std::tie(cImg, raport) = compress(img, getQuantizer((Quantizers)par->quantizer), par->width, par->height, par->eps, par->n);
  if(par->raport)
    std::cout << raport;
  RGBImage decompressed = decompress(cImg);
  decompressed.saveToFile(par->saveto);
}
