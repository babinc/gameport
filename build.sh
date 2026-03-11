#!/bin/sh
set -e
cmake -B build -DCMAKE_BUILD_TYPE="${1:-Debug}"
cmake --build build
echo "Built: ./build/open-game-portal"
