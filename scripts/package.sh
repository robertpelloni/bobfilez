#!/bin/bash
set -e

cd "$(dirname "$0")/.."

# Read version from VERSION.md
VERSION=$(cat VERSION.md | tr -d '[:space:]')

echo "========================================"
echo "filez Packaging Script v$VERSION"
echo "========================================"

# Detect platform
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
    ARCH=$(uname -m)
    if [[ "$ARCH" == "arm64" ]]; then
        ARCHIVE_SUFFIX="macos-arm64"
    else
        ARCHIVE_SUFFIX="macos-x64"
    fi
else
    PLATFORM="linux"
    ARCHIVE_SUFFIX="linux-x64"
fi

# Clean previous builds
echo "[1/5] Cleaning previous builds..."
rm -rf dist build

# Create build directory and run cmake
echo "[2/5] Configuring CMake (Release)..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

# Build the project
echo "[3/5] Building project..."
cmake --build build --config Release
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

# Create dist directory structure
echo "[4/5] Creating distribution..."
mkdir -p dist/docs

# Copy essential files
cp build/cli/fo_cli dist/
cp README.md dist/
cp LICENSE.txt dist/
if [ -f docs/USER_MANUAL.md ]; then
    cp docs/USER_MANUAL.md dist/docs/
fi

# Verify files exist
if [ ! -f dist/fo_cli ]; then
    echo "ERROR: fo_cli not found in build output"
    exit 1
fi

# Make executable
chmod +x dist/fo_cli

# Create archive
echo "[5/5] Creating archive..."
ARCHIVE_NAME="filez-${VERSION}-${ARCHIVE_SUFFIX}.zip"
cd dist
zip -r "../$ARCHIVE_NAME" .
cd ..

echo "========================================"
echo "SUCCESS: Created $ARCHIVE_NAME"
echo "========================================"
ls -lh "$ARCHIVE_NAME"
