cmake_minimum_required(VERSION 3.10)

project(LC3_VM VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Machine main.cpp lc3.hpp memory.hpp)
