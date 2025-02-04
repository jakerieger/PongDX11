cmake_minimum_required(VERSION 3.29)
project(PongDX11)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(PongDX11 WIN32
        main.cpp
        StepTimer.h
        DeviceResources.h
        DeviceResources.cpp
        Game.cpp
        Game.h
)

target_precompile_headers(PongDX11 PRIVATE pch.h)
target_include_directories(PongDX11 PRIVATE ${CMAKE_SOURCE_DIR})
target_sources(PongDX11 PRIVATE pch.cpp)

add_subdirectory(${CMAKE_SOURCE_DIR}/DirectXTK ${CMAKE_BINARY_DIR}/bin/CMake/DirectXTK)
target_link_libraries(PongDX11 PRIVATE
        d3d11.lib
        dxgi.lib
        dxguid.lib
        d2d1.lib
        dwrite.lib
        uuid.lib
        kernel32.lib
        user32.lib
        comdlg32.lib
        advapi32.lib
        shell32.lib
        ole32.lib
        oleaut32.lib
)
target_link_libraries(PongDX11 PRIVATE DirectXTK)

add_custom_command(TARGET PongDX11 PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data)