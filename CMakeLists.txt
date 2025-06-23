cmake_minimum_required(VERSION 3.29)
project(ZX16_System_Simulator)

set(CMAKE_CXX_STANDARD 20)

add_executable(ZX16_System_Simulator main.cpp
        Instruction.cpp
        Instruction.h
        ZX16_Simulator.cpp
        ZX16_Simulator.h)
