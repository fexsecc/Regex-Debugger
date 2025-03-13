#!/bin/bash

# Get current dir of script and cd into it
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR"

# clone vcpkg and build with cmake
git clone https://github.com/microsoft/vcpkg.git vcpkg
cmake --preset=default
