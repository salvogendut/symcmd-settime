#!/bin/bash
#
# Build HTTP Time Client (Direct syscall) for SymbOS
#

CC=~/Dev/LEISURE/scc/bin/cc
SRC_DIR=./src
OUTPUT_DIR=./build

echo "Building HTTP Time Client (Direct syscall)..."

# Create output directory
mkdir -p $OUTPUT_DIR

# Compile
echo "Compiling C source..."
$CC -c $SRC_DIR/timeclient-direct.c -I./include -o $OUTPUT_DIR/timeclient-direct.o || exit 1
$CC -c $SRC_DIR/config.c -I./include -o $OUTPUT_DIR/config.o || exit 1

echo "Assembling..."
$CC -c $SRC_DIR/timeset_direct.s -o $OUTPUT_DIR/timeset_direct.o || exit 1

echo "Linking..."
$CC $OUTPUT_DIR/timeclient-direct.o \
    $OUTPUT_DIR/config.o \
    $OUTPUT_DIR/timeset_direct.o \
    -I./include \
    -lnet \
    -o $OUTPUT_DIR/settime.com || exit 1

if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful!"
    echo "Output: $OUTPUT_DIR/settime.com"
    ls -lh $OUTPUT_DIR/settime.com
else
    echo ""
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Done."
