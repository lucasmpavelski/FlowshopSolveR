#!/bin/bash

cd build
cmake -DCMAKE_BUILD_TYPE=Debug -GNinja ..
make refine
cd ..
./build/main/refine
