#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

# URL to the ArrayFire installer
URL="https://arrayfire.gateway.scarf.sh/linux/3.9.0/ArrayFire.sh"

# Local file name for the installer
INSTALLER="ArrayFire.sh"

# Directory for installation
INSTALL_DIR="/opt/arrayfire/ArrayFire-3.9.0-Linux/"

# Download the installer
echo "Downloading ArrayFire installer..."
wget -O $INSTALLER $URL
if [[ $? -ne 0 ]]; then
    echo "Failed to download ArrayFire installer."
    exit 1
fi

# Make the installer executable
echo "Making the installer executable..."
chmod +x $INSTALLER

# Create installation directory
echo "Creating installation directory..."
sudo mkdir -p $INSTALL_DIR
if [[ $? -ne 0 ]]; then
    echo "Failed to create installation directory."
    exit 1
fi

# Run the installer
echo "Running the installer..."
sudo ./$INSTALLER --include-subdir --prefix=/opt
if [[ $? -ne 0 ]]; then
    echo "Failed to install ArrayFire."
    exit 1
fi

# Setup environment variables
echo "Setting up environment variables..."

# Append ArrayFire CMake directory to CMAKE_PREFIX_PATH
echo "export CMAKE_PREFIX_PATH=\"$INSTALL_DIR:\$CMAKE_PREFIX_PATH\"" >> ~/.bashrc 

# Append the ArrayFire lib directory to LD_LIBRARY_PATH for runtime linking
echo "export AF_LIBRARY_PATH=\"$INSTALL_DIR/lib64:\$LD_LIBRARY_PATH\"" >> ~/.bashrc

echo $INSTALL_DIR/lib64 > /etc/ld.so.conf.d/arrayfire.conf
sudo ldconfig

# Source bashrc to update current session
source ~/.bashrc

# Clean up
echo "Cleaning up..."
rm $INSTALLER

echo "ArrayFire installed successfully to $INSTALL_DIR."

echo "Installing other dependencies..."
apt install build-essential libfreeimage3 libfontconfig1 libglu1-mesa

echo "Testing ArrayFire installation:"
cp -r $INSTALL_DIR/share/ArrayFire/examples /tmp/examples
cd /tmp/examples
mkdir build
cd build
cmake ..
make -j8

./helloworld/helloworld_cpu
./helloworld/helloworld_cuda
./helloworld/helloworld_opencl
./helloworld/helloworld_oneapi

echo "Environment set up. Please reopen your terminal or run 'source ~/.bashrc' to refresh settings."
