#include "Compressor.hpp"
#include "Debug.hpp"
#include "ProgramParameters.hpp"
#include "RGBImage.hpp"
#include "nanoflann.hpp"

#include "boost/program_options.hpp"
#include <iostream>

enum class FileType
{
  NOT_SUPPORTED, PPM, QUANT, LAST
};

int fileTypeMap(FileType a, FileType b)
{
  return (int)a*(int)FileType::LAST+(int)b;
}

FileType getFileType(std::string path)
{
  std::string fileExt;
  for(int i = path.size()-1; i >= 0; i--)
  {
    if(path[i] == '/')
      return FileType::NOT_SUPPORTED;

    if(path[i] == '.')
    {
      fileExt = path.substr(i);
      break;
    }
  }
  if(fileExt == ".quant")
    return FileType::QUANT;
  if(fileExt == ".ppm")
    return FileType::PPM;

  return FileType::NOT_SUPPORTED;
}

int main(int argc, char **argv) 
{
  namespace po = boost::program_options;
  po::options_description desc("Options");
  auto par = getParams();
  desc.add_options()
    ("help", "Print help messages")
    (",n", po::value<int>(&par->n)->default_value(10), "bits per codevector")
    (",e", po::value<float>(&par->eps)->default_value(0.000001), "eps parameter for quantization algorithm")
    (",w", po::value<int>(&par->width)->default_value(2), "Width of block")
    (",h", po::value<int>(&par->height)->default_value(2), "Height of block") 
    ("file", po::value<std::string>(&par->file)->required(), "File to compress/decompress")
    ("saveto,o", po::value<std::string>(&par->saveto)->required(), "Save to")
    (",r", po::value<bool>(&par->raport)->default_value(false), "Print raport to std::out")
    ("quantizer,q", po::value<int>(&par->quantizer)->default_value((int)Quantizers::LBG), "Pick quantizer")
    ("c,colorspace", po::value<int>(&par->colorspace)->default_value((int)ColorSpaces::SCALED), "Pick ColorSpace");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("help")) 
  {
    std::cout << desc << std::endl;
    return 0;
  }
  vm.notify();

  FileType fromType = getFileType(par->file);
  FileType toType = getFileType(par->saveto);

  auto runCompression = [&]()
  {
    RGBImage img(par->file);
    auto result = CompressedImage::compress
        (img, (Quantizers)par->quantizer, (ColorSpaces)par->colorspace, par->width, par->height, par->eps, par->n);
    if (par->raport)
      std::cout << result.second;
    return result.first;
  };

  auto runDecompression = [&](auto &cImg)
  {
    RGBImage decompressed = CompressedImage::decompress(cImg);
    decompressed.saveToFile(par->saveto);
  };

  if(fromType == FileType::PPM && toType == FileType::PPM)
  {
    auto cImg = runCompression();
    runDecompression(cImg);
  }
  else if(fromType == FileType::QUANT && toType == FileType::PPM)
  {
    CompressedImage cImg;
    cImg.loadFromFile(par->file);
    runDecompression(cImg);
  }
  else if(fromType == FileType::PPM && toType == FileType::QUANT)
  {
    auto cImg = runCompression();
    cImg.saveToFile(par->saveto);
  }
  else
  {
    std::cerr << "File type not supported" << std::endl;
    return 1;
  }
}
