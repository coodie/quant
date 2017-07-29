#include "gtest/gtest.h"
#include "Compressor.hpp"
#include "Debug.hpp"

TEST(compressor_test, something)
{
  RGBImage testImg;
  testImg.img =
    {
      {'a','b','c'}, {'d','e','f'}, {'g', 'h', 'i'}, {'j', 'k', 'l'},
      {'a','b','c'}, {'d','e','f'}, {'g', 'h', 'i'}, {'j', 'k', 'l'},
      {'a','b','c'}, {'d','e','f'}, {'g', 'h', 'i'}, {'j', 'k', 'l'},
      {'a','b','c'}, {'d','e','f'}, {'g', 'h', 'i'}, {'j', 'k', 'l'}
    };
  testImg.xSize = 4;
  testImg.ySize = 4;
  ColorSpacePtr cs = getColorSpace(ColorSpaces::NORMAL);
  {
    int w = 1;
    int h = 1;

    auto blocks = getBlocksAsVectorsFromImage(testImg, w, h, cs);
    auto converted = vectorsToCharVectorsColorSpaced(blocks, cs);
    auto expected = getImageFromVectors(converted, testImg.xSize, testImg.ySize, w, h);

    EXPECT_EQ(expected.img, testImg.img);
  }

  {
    int w = 2;
    int h = 2;

    auto blocks = getBlocksAsVectorsFromImage(testImg, w, h, cs);
    auto converted = vectorsToCharVectorsColorSpaced(blocks, cs);
    auto expected = getImageFromVectors(converted, testImg.xSize, testImg.ySize, w, h);

    EXPECT_EQ(expected.img, testImg.img);
  }

   {
     int w = 1;
     int h = 3;

     auto blocks = getBlocksAsVectorsFromImage(testImg, w, h, cs);
     auto converted = vectorsToCharVectorsColorSpaced(blocks, cs);
     auto expected = getImageFromVectors(converted, testImg.xSize, testImg.ySize, w, h);

     EXPECT_EQ(expected.img, testImg.img);
   }

   {
     int w = 2;
     int h = 4;

     auto blocks = getBlocksAsVectorsFromImage(testImg, w, h, cs);
     auto converted = vectorsToCharVectorsColorSpaced(blocks, cs);
     auto expected = getImageFromVectors(converted, testImg.xSize, testImg.ySize, w, h);

     EXPECT_EQ(expected.img, testImg.img);
   }
}
