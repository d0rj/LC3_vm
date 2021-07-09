cmake_minimum_required(VERSION 3.10)

project(LC3_VM VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB LC3_VM_SRC
     "src/*.h"
     "src/*.cpp"
)

add_executable(Machine main.cpp ${LC3_VM_SRC})
