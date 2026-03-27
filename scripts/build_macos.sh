#!/bin/sh
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

# Source .env if present (for CODESIGN_IDENTITY)
if [ -f "$PROJECT_DIR/.env" ]; then
    set -a
    . "$PROJECT_DIR/.env"
    set +a
fi

# Find Qt6 prefix (Homebrew)
QT_PREFIX="$(brew --prefix qt@6 2>/dev/null || true)"
if [ -z "$QT_PREFIX" ]; then
    echo "Error: Qt6 not found. Install with: brew install qt@6"
    exit 1
fi

# Check if Qt supports both architectures for universal build
QT_ARCHS="$(lipo -archs "$QT_PREFIX/lib/QtCore.framework/QtCore" 2>/dev/null || true)"
if echo "$QT_ARCHS" | grep -q "x86_64" && echo "$QT_ARCHS" | grep -q "arm64"; then
    ARCHS="x86_64;arm64"
    echo "==> Configuring (universal binary: x86_64 + arm64)"
else
    ARCHS="$(uname -m)"
    echo "==> Configuring (native: $ARCHS — Qt is not universal)"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake "$PROJECT_DIR" \
    -DCMAKE_PREFIX_PATH="$QT_PREFIX" \
    -DCMAKE_OSX_ARCHITECTURES="$ARCHS" \
    -DCMAKE_BUILD_TYPE=Release

echo "==> Building"
make -j"$(sysctl -n hw.ncpu)"

echo "==> Running tests"
ctest --output-on-failure

echo "==> Verifying binary"
file "$BUILD_DIR/src/gamepad-tool.app/Contents/MacOS/gamepad-tool"

if [ -n "$CODESIGN_IDENTITY" ] && [ "$CODESIGN_IDENTITY" != "-" ]; then
    echo "==> Signed with: $CODESIGN_IDENTITY"
else
    echo "==> Ad-hoc signed (set CODESIGN_IDENTITY for Developer ID signing)"
fi

echo "==> Done: $BUILD_DIR/src/gamepad-tool.app"
