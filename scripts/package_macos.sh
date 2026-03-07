#!/bin/bash
set -e

cd "$(dirname "$0")/.."

# Read version from VERSION.md
VERSION=$(cat VERSION.md | tr -d '[:space:]')

echo "========================================"
echo "filez macOS DMG Packaging Script v$VERSION"
echo "========================================"

# Required tools check
if ! command -v hdiutil &> /dev/null; then
    echo "ERROR: hdiutil not found. This script requires macOS to run."
    exit 1
fi

# Clean previous builds
echo "[1/6] Cleaning previous builds..."
rm -rf dist_macos build_macos
mkdir -p dist_macos

# Configure CMake
echo "[2/6] Configuring CMake (Release)..."
TOOLCHAIN_ARGS=""
if [ -f "vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    TOOLCHAIN_ARGS="-DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake"
fi

cmake -S . -B build_macos -G Ninja -DCMAKE_BUILD_TYPE=Release $TOOLCHAIN_ARGS
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

# Build project
echo "[3/6] Building project..."
cmake --build build_macos --config Release
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

# Create App bundle structure
echo "[4/6] Creating filez.app bundle..."
APP_BUNDLE="dist_macos/filez.app"
mkdir -p "$APP_BUNDLE/Contents/MacOS"
mkdir -p "$APP_BUNDLE/Contents/Resources"

cp build_macos/cli/fo_cli "$APP_BUNDLE/Contents/MacOS/filez"
chmod +x "$APP_BUNDLE/Contents/MacOS/filez"

# Create Info.plist
cat <<EOF > "$APP_BUNDLE/Contents/Info.plist"
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>filez</string>
    <key>CFBundleIdentifier</key>
    <string>com.bobfilez.fo_cli</string>
    <key>CFBundleName</key>
    <string>filez</string>
    <key>CFBundleVersion</key>
    <string>${VERSION}</string>
    <key>CFBundleShortVersionString</key>
    <string>${VERSION}</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.15</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>????</string>
</dict>
</plist>
EOF

# Code signing (Ad-Hoc)
echo "[5/6] Ad-Hoc Code Signing..."
codesign --force --deep --sign - "$APP_BUNDLE"

# Create DMG
echo "[6/6] Creating DMG archive..."
DMG_NAME="filez-${VERSION}-macos.dmg"
DMG_VOLNAME="filez v${VERSION}"

# Create a staging directory for the DMG contents
STAGING_DIR="dist_macos/dmg_staging"
mkdir -p "$STAGING_DIR"
cp -r "$APP_BUNDLE" "$STAGING_DIR/"
ln -s /Applications "$STAGING_DIR/Applications"

# Provide fallback for README/LICENSE
cp README.md "$STAGING_DIR/"
cp LICENSE.txt "$STAGING_DIR/"

hdiutil create -volname "$DMG_VOLNAME" -srcfolder "$STAGING_DIR" -ov -format UDZO "dist_macos/$DMG_NAME"

echo "========================================"
echo "SUCCESS: Created dist_macos/$DMG_NAME"
echo "========================================"
ls -lh "dist_macos/$DMG_NAME"
cd ..
