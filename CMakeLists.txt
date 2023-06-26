cmake_minimum_required(VERSION 3.24)
project(chr_dev C)

set(CMAKE_C_STANDARD 99)

add_executable(chr_dev main.c chr_dev.c chr_dev.h)
