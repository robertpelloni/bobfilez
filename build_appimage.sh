#!/bin/bash
set -e

# Compile the core CLI for the AppImage
export CC=gcc
export CXX=g++
rm -rf build_appimage
mkdir -p build_appimage
cd build_appimage

cmake -S .. -B . -DFO_BUILD_CLI=ON -DFO_BUILD_GUI=OFF -DFO_BUILD_OMNI=OFF -DFO_BUILD_QT_DEMO=OFF -DFO_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$PWD/../vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Release --target fo_cli

cd ..

# Download linuxdeploy
wget -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
chmod +x linuxdeploy-x86_64.AppImage

# Create AppDir structure
rm -rf AppDir
mkdir -p AppDir/usr/bin
mkdir -p AppDir/usr/share/applications
mkdir -p AppDir/usr/share/icons/hicolor/256x256/apps

# Copy the CLI executable
cp build_appimage/cli/fo_cli AppDir/usr/bin/bobfilez

# Create a minimal .desktop file
cat << 'DESKTOP' > AppDir/usr/share/applications/bobfilez.desktop
[Desktop Entry]
Type=Application
Name=BobFilez
Comment=Cross-platform file organization and deduplication engine
Exec=bobfilez %F
Icon=bobfilez
Categories=Utility;System;
Terminal=true
DESKTOP

# Create a dummy icon since we don't have one readily available here
touch AppDir/usr/share/icons/hicolor/256x256/apps/bobfilez.png

# Run linuxdeploy to bundle dependencies and generate the AppImage
./linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage
