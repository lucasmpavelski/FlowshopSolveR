#!/bin/bash

cd build
cmake -DCMAKE_BUILD_TYPE=Release -Gninja ..
make refine
cd ..
./build/main/refine
