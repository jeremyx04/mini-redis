#!/bin/bash

handle_error() {
    echo "Error: $1"
    exit 1
}

echo "Cleaning up old build..."
rm -f main CMakeCache.txt || handle_error "Failed to clean up old files"
rm -f ./tests/resp_test*
echo "Building project..."
cmake ./
make || handle_error "Build failed"

echo "Complete!"
