# 1. Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# 2. Name your project
project(TerrainGenerator VERSION 1.0)

# 3. Require modern C++ (C++17 is great for modern random/math features)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 4. Define your executable and add ALL your .cpp files here
add_executable(TerrainGenerator
    src/main.cpp
    src/Map/Generator.cpp
    src/IO/ImageExport.cpp
    src/Simulation/Tectonic.cpp
)

# 5. Tell CMake where your header files live
target_include_directories(TerrainGenerator PUBLIC include third_party)