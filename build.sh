#!/usr/bin/env bash
set -e

cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release --log-level=WARNING
cmake --build build -j 8

./build/Chess.exe