#!/bin/bash
# URL to the ArrayFire installer
URL="https://arrayfire.gateway.scarf.sh/linux/3.9.0/ArrayFire.sh"

# Local file name for the installer
INSTALLER="ArrayFire.sh"

# Directory for installation
INSTALL_DIR="/opt/arrayfire"

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
sudo ./$INSTALLER --prefix=$INSTALL_DIR
if [[ $? -ne 0 ]]; then
    echo "Failed to install ArrayFire."
    exit 1
fi

# Clean up
echo "Cleaning up..."
rm $INSTALLER

echo "ArrayFire installed successfully to $INSTALL_DIR."
