#!/bin/bash
#!/bin/bash

# This script sequentially executes other scripts: installation, building, and running.

echo "Starting installation..."
./installAF.sh
if [ $? -ne 0 ]; then
    echo "Installation failed."
    exit 1
fi

echo "Building project..."
./build.sh
if [ $? -ne 0 ]; then
    echo "Build failed."
    exit 1
fi

echo "Running all components..."
./runAll.sh
if [ $? -ne 0 ]; then
    echo "Execution failed."
    exit 1
fi

echo "All steps completed successfully."

