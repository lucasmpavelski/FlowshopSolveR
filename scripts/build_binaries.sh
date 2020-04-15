rm -r build
mkdir build
cd build
export CC=clang 
export CXX=clang++ 
cmake -DCMAKE_BUILD_TYPE=Release -GNinja ..
ninja
cd ..
