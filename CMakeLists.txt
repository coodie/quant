cmake_minimum_required(VERSION 3.5.1)
project (quant)

include_directories(include)
add_compile_options(--std=c++14)

set(src_files
  src/main.cpp
  src/RGBImage.cpp
  src/Compressor.cpp)

set(used_boost_libs
  program_options)

find_package(Boost COMPONENTS ${used_boost_libs} REQUIRED) 
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
    add_executable(quant ${src_files})
    target_link_libraries(quant ${Boost_LIBRARIES})
endif()

