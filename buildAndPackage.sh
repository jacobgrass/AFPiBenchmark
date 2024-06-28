#!/bin/bash

# Define output directory and zip file name
OUTPUT_DIR="output"
ZIP_FILE="AFPiBenchMark.zip"

# Clean previous build artifacts
rm -rf build $OUTPUT_DIR $ZIP_FILE

# Create build directory
mkdir outbuild
cd outbuild

# Run cmake to configure the project
cmake ..

# Build all targets
make

# Create output directory for distribution
mkdir -p ../$OUTPUT_DIR

# Copy binaries and dependencies to the output directory
if [ -f AFPiBenchMark_cpu ]; then
  cp AFPiBenchMark_cpu ../$OUTPUT_DIR/
  cp $(ldd AFPiBenchMark_cpu | grep '=>' | awk '{print $3}') ../$OUTPUT_DIR/
fi

if [ -f AFPiBenchMark_cuda ]; then
  cp AFPiBenchMark_cuda ../$OUTPUT_DIR/
  cp $(ldd AFPiBenchMark_cuda | grep '=>' | awk '{print $3}') ../$OUTPUT_DIR/
fi

if [ -f AFPiBenchMark_opencl ]; then
  cp AFPiBenchMark_opencl ../$OUTPUT_DIR/
  cp $(ldd AFPiBenchMark_opencl | grep '=>' | awk '{print $3}') ../$OUTPUT_DIR/
fi

cp runAll.sh ../$OUTPUT_DIR/

# Go back to root directory to create the zip file
cd ..

# Create a zip archive from the output directory
zip -r $ZIP_FILE $OUTPUT_DIR

echo "Build and package complete. Distribution package created as $ZIP_FILE"

