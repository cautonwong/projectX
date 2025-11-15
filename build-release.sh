#!/bin/bash
# =============================================================================
# Cross-Platform Go Build Script using 'xgo'
#
# This script uses 'xgo' to handle CGO cross-compilation for all platforms.
# It ensures a consistent and reliable build process.
#
# Prerequisites:
#   1. Go (to install xgo)
#   2. Docker (xgo runs inside it)
#   3. 'xgo' (run: go install github.com/crazy-max/xgo@latest)
#
# Usage:
#   ./build-release.sh [version]
#   (Defaults to latest git tag if version is omitted)
# =============================================================================

set -e

# --- Configuration ---
APP_NAME="edge-processor"
RELEASE_DIR="release"
MAIN_PACKAGE="go/cmd/main.go" # Build the package in the current directory

# Define platforms to build for
TARGETS="windows/amd64,linux/amd64,linux/arm64,darwin/amd64,darwin/arm64"

# Tags needed for mattn/go-sqlite3 cross-compilation
BUILD_TAGS="sqlite_omit_load_extension"

# --- Script Logic ---

# 1. Check dependencies
if ! command -v xgo &> /dev/null; then
    echo "Error: 'xgo' not found in PATH."
    echo "Please install it first: go install github.com/crazy-max/xgo@latest"
    exit 1
fi
if ! command -v docker &> /dev/null || ! docker info &> /dev/null; then
    echo "Error: Docker is not running or not installed."
    echo "xgo requires a running Docker daemon."
    exit 1
fi

# 2. Determine version
VERSION=${1}
if [ -z "$VERSION" ]; then
    echo "==> Version not specified, using latest git tag..."
    git fetch --tags --quiet
    VERSION=$(git describe --tags --abbrev=0 2>/dev/null)
    if [ -z "$VERSION" ]; then
        echo "Error: No version specified and no git tags found."
        exit 1
    fi
fi
echo "==> Building ${APP_NAME} version: ${VERSION}"

# 3. Clean up previous release directory
echo "==> Cleaning up old release directory..."
rm -rf "${RELEASE_DIR}"
mkdir -p "${RELEASE_DIR}"

# 4. Define Linker Flags (for optimization and version injection)
LDFLAGS=(
    "-s -w"
    "-X main.version=${VERSION}"
)
LD_FLAGS_STR="${LDFLAGS[*]}"
echo "==> Using LDFLAGS: ${LD_FLAGS_STR}"
echo "==> Using Build Tags: ${BUILD_TAGS}"

# 5. Run xgo
echo "==> Starting 'xgo' build process (this may take a few minutes)..."
xgo \
    --pkg-version="${VERSION}" \
    --targets="${TARGETS}" \
    --out="${RELEASE_DIR}/${APP_NAME}" \
    --ldflags="${LD_FLAGS_STR}" \
    --tags="${BUILD_TAGS}" \
    "${MAIN_PACKAGE}"

echo "==> 'xgo' build complete."

# 6. Post-process: Package builds into archives
echo "==> Packaging archives..."
cd "${RELEASE_DIR}"

# Find all build output directories (e.g., edge-processor-v1.0.0-linux-amd64)
for dir in ${APP_NAME}-${VERSION}-*; do
    if [ -d "${dir}" ]; then
        echo "    -> Processing ${dir}"
        
        # Copy config and README into the build directory
        cp ../config.yml "${dir}/config.example.yml"
        if [ -f "../README.md" ]; then
            cp ../README.md "${dir}/"
        fi

        # Create archive
        if [[ "${dir}" == *"windows"* ]]; then
            zip -r "${dir}.zip" "${dir}"
        else
            tar -czf "${dir}.tar.gz" "${dir}"
        fi
        
        # Clean up the directory
        rm -rf "${dir}"
    fi
done

cd ..
echo "==> All builds completed and packaged successfully!"
echo "==> Release artifacts are in: ${RELEASE_DIR}"
ls -l "${RELEASE_DIR}"