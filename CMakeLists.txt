cmake_minimum_required(VERSION 3.28)
project(CurseWork)

set(CMAKE_CXX_STANDARD 23)

add_executable(CurseWork server.cpp)
add_subdirectory(client)
