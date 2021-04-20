# cmake_minimum_required(VERSION <specify CMake version here>)
cmake_minimum_required(VERSION 3.7.2)
project(BS21 C)

set(CMAKE_C_STANDARD 99)

add_executable(BS21 main.c main.h keyValStore.c keyValStore.h sub.c sub.h)