#!/bin/bash

BUILD_TYPE="${1:-Release}"
ENABLE_VM="${2:-ON}"
ENABLE_GC="${3:-ON}"
ENABLE_OPT="${4:-ON}"

echo "==================================="
echo "Bald++ Build Configuration"
echo "==================================="
echo "Build type:    $BUILD_TYPE"
echo "VM:            $ENABLE_VM"
echo "GC:            $ENABLE_GC"
echo "Optimizations: $ENABLE_OPT"
echo "==================================="

mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DBALD_ENABLE_VM=$ENABLE_VM \
      -DBALD_ENABLE_GC=$ENABLE_GC \
      -DBALD_ENABLE_OPTIMIZATIONS=$ENABLE_OPT \
      ..

make -j$(nproc)

echo ""
echo "Build complete! Executable: build/bald_interpreter"
echo ""
echo "Run with: ./build/bald_interpreter [--vm|--interpreter] [--stats] <file.bald>"

