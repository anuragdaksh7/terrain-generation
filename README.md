# Create a build folder to keep your root clean
mkdir build
cd build

# Tell CMake to generate the Makefiles based on your CMakeLists.txt
cmake -G "MinGW Makefiles" ..

# Compile the actual project
make