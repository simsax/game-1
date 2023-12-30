#!/bin/bash

cd build
cmake -B . -S ..

if [ $? -ne 0 ]; then
    echo "CMake failed"
    exit 1
fi

make -j8

if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

./game-1
