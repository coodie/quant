cmake_minimum_required(VERSION 3.2.2)
project (quant)

####### Compiler options
add_compile_options(--std=c++14 -O3 -g -Wall)

####### Project options
include_directories(include)
include_directories(include/external)

set(src_files
  src/main.cpp
  src/RGBImage.cpp
  src/Compressor.cpp
  src/KDTree.cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

####### Add Boost
set(used_boost_libs
  program_options)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS ${used_boost_libs} REQUIRED)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(quant ${src_files})
    target_link_libraries(quant ${Boost_LIBRARIES})
endif()

####### Add GTest
add_subdirectory(gtest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCEDIR})

add_executable(runtest src/test.cpp)
target_link_libraries(runtest gtest gtest_main)
add_test(runtest runtest)
