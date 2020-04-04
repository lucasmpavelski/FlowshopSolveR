#!/bin/bash

cd build
cmake ..
make refine
cd ..
./build/main/refine
