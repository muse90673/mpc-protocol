#!/bin/bash
set -e

# Get script directory
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
PROJECT_ROOT=$(dirname "$SCRIPT_DIR")
DIST_DIR="$PROJECT_ROOT/dist"

# Create dist directory
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

echo -e "\nBuilding all packages..."

# Build Windows exe version
echo -e "\nBuilding Windows exe version..."
cd "$PROJECT_ROOT/windows/exe"
./build.sh
zip -r "$DIST_DIR/mpc-protocol-windows-exe.zip" \
  mpc-protocol.exe \
  mpc-protocol-register.bat \
  mpc-protocol-deregister.bat


# Package Windows PowerShell version
echo -e "\nPackaging Windows PowerShell version..."
cd "$PROJECT_ROOT/windows/ps"
zip -r "$DIST_DIR/mpc-protocol-windows-powershell.zip" \
    mpc-protocol.ps1 \
    mpc-protocol-register.ps1 \
    mpc-protocol-deregister.ps1

# Package Windows batch version
echo -e "\nPackaging Windows batch version..."
cd "$PROJECT_ROOT/windows/bat"
zip -r "$DIST_DIR/mpc-protocol-windows-bat.zip" \
    mpc-protocol.bat \
    mpc-protocol-register.bat \
    mpc-protocol-deregister.bat

echo -e "\nBuild complete! Packages are in the dist directory:"
ls -lh "$DIST_DIR"