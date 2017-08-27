cmake_minimum_required(VERSION 3.2.2)
project (quant)

####### Compiler options
add_compile_options(--std=c++14 -Wall -Wno-missing-braces)

# CMake automatically adds -g to Debug build
# CMake automatically adds -O3 and -NDEBUG to Release build
set(CMAKE_CXX_FLAGS_RELEASE 
    "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math")



if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release
RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

####### Project options
include_directories(include)
include_directories(include/external)

set(src_files
  src/RGBImage.cpp
  src/Compressor.cpp
  src/KDTree.cpp
  src/ColorSpace.cpp
  src/Quantizer.cpp
  src/ProgramParameters.cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

####### Add Boost
set(used_boost_libs
  program_options)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS ${used_boost_libs} REQUIRED)

if(Boost_FOUND)
    add_library(quantsrc STATIC ${src_files})
    target_link_libraries(quantsrc ${Boost_LIBRARIES})

    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(quant src/main.cpp)
    target_link_libraries(quant quantsrc)
endif()

####### Add GTest
add_subdirectory(gtest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCEDIR})

add_executable(runtest src/test.cpp)
target_link_libraries(runtest gtest gtest_main quantsrc)
add_test(runtest runtest)
