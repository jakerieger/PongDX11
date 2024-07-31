cmake_minimum_required(VERSION 3.29)
project(PongDX11)

set(CMAKE_CXX_STANDARD 20)

add_executable(PongDX11 WIN32 main.cpp
        Game.cpp
        Game.h)

target_precompile_headers(PongDX11 PRIVATE pch.h)
target_include_directories(PongDX11 PRIVATE ${CMAKE_SOURCE_DIR})
target_sources(PongDX11 PRIVATE pch.cpp)
