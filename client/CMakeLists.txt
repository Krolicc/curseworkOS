cmake_minimum_required(VERSION 3.28)
project(CurseWork_client)

set(CMAKE_CXX_STANDARD 23)

add_executable(CurseWork_client client.cpp)

#add_library(CurseWork_client client.cpp)

set_target_properties(
        CurseWork_client PROPERTIES
        LANGUAGES CXX
        LINKER_LANGUAGE CXX
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        VERSION 1.0
        DESCRIPTION "client request")