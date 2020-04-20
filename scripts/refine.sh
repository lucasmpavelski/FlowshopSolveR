#!/bin/bash

cd build
cmake -DCMAKE_BUILD_TYPE=Release -GNinja ..
make refine
cd ..
./build/main/refine
