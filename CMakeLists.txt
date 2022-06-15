cmake_minimum_required(VERSION 3.22)
project(comp_wet3)

set(CMAKE_CXX_STANDARD 14)

add_executable(comp_wet3 dflow_main.c dflow_calc.cpp dflow_calc.h dflow_calc.cpp)
