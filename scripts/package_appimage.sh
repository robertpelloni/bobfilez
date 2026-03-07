#!/bin/bash
set -e

cd "$(dirname "$0")/.."

# Read version from VERSION.md
VERSION=$(cat VERSION.md | tr -d '[:space:]')

echo "========================================"
echo "filez AppImage Packaging Script v$VERSION"
echo "========================================"

# Clean previous builds
echo "[1/6] Cleaning previous builds..."
rm -rf dist_appimage build_appimage
mkdir -p dist_appimage

# Create build directory and run cmake
echo "[2/6] Configuring CMake (Release)..."
# Adding vcpkg toolchain if available
TOOLCHAIN_ARGS=""
if [ -f "vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    TOOLCHAIN_ARGS="-DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake"
fi

cmake -S . -B build_appimage -DCMAKE_BUILD_TYPE=Release $TOOLCHAIN_ARGS
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

# Build the project
echo "[3/6] Building project..."
cmake --build build_appimage --config Release
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

# Create AppDir structure
echo "[4/6] Creating AppDir..."
APPDIR="dist_appimage/filez.AppDir"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"

# Copy binary
cp build_appimage/cli/fo_cli "$APPDIR/usr/bin/"

# Create desktop file
cat <<EOF > "$APPDIR/usr/share/applications/filez.desktop"
[Desktop Entry]
Name=filez
Exec=fo_cli
Icon=filez
Type=Application
Categories=Utility;
EOF

# Create a dummy icon (or use a real one if exists)
cat <<EOF > "$APPDIR/filez.svg"
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100">
  <rect width="100" height="100" fill="#007ACC"/>
  <text x="50" y="55" font-family="Arial" font-size="24" fill="white" text-anchor="middle">filez</text>
</svg>
EOF
cp "$APPDIR/filez.svg" "$APPDIR/usr/share/icons/hicolor/256x256/apps/filez.svg"
cp "$APPDIR/filez.svg" "$APPDIR/.DirIcon"

# Create symbolic link for AppRun
cat <<EOF > "$APPDIR/AppRun"
#!/bin/sh
HERE="\$(dirname "\$(readlink -f "\${0}")")"
export PATH="\${HERE}/usr/bin:\${PATH}"
export LD_LIBRARY_PATH="\${HERE}/usr/lib:\${LD_LIBRARY_PATH}"
exec "\${HERE}/usr/bin/fo_cli" "\$@"
EOF
chmod +x "$APPDIR/AppRun"

echo "[5/6] Downloading linuxdeploy and appimagetool..."
cd dist_appimage
if [ ! -f appimagetool-x86_64.AppImage ]; then
    wget -q https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x appimagetool-x86_64.AppImage
fi

echo "[6/6] Generating AppImage..."
# Extract appimagetool to run it without FUSE requirement
./appimagetool-x86_64.AppImage --appimage-extract
./squashfs-root/AppRun filez.AppDir "filez-${VERSION}-linux-x86_64.AppImage"

echo "========================================"
echo "SUCCESS: Created filez-${VERSION}-linux-x86_64.AppImage"
echo "========================================"
ls -lh "filez-${VERSION}-linux-x86_64.AppImage"
cd ..
