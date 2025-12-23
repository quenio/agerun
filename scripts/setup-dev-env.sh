#!/bin/bash
# Development Environment Setup Check
# This script verifies that all required tools are installed with the correct versions.

set -e

echo "============================================"
echo "AgeRun Development Environment Check"
echo "============================================"
echo ""

HAS_ERROR=0

# Check GCC 13
echo "Checking for gcc-13..."
if ! command -v gcc-13 &> /dev/null; then
    echo "❌ gcc-13 not found"
    echo "   Install with: sudo apt install gcc-13 g++-13"
    HAS_ERROR=1
else
    GCC_VERSION=$(gcc-13 --version | head -1)
    echo "✅ gcc-13 found: $GCC_VERSION"
fi

# Check Zig 0.14.1
echo ""
echo "Checking for Zig 0.14.1..."
if ! command -v zig &> /dev/null; then
    echo "❌ Zig not found"
    echo "   Install Zig 0.14.1 from: https://ziglang.org/download/"
    echo "   Or use asdf: asdf install zig 0.14.1"
    HAS_ERROR=1
else
    ZIG_VERSION=$(zig version)
    if [ "$ZIG_VERSION" != "0.14.1" ]; then
        echo "⚠️  Zig version $ZIG_VERSION found, but 0.14.1 is required"
        echo "   Download from: https://ziglang.org/download/0.14.1/"
        HAS_ERROR=1
    else
        echo "✅ Zig 0.14.1 found"
    fi
fi

# Check Make
echo ""
echo "Checking for make..."
if ! command -v make &> /dev/null; then
    echo "❌ make not found"
    echo "   Install with: sudo apt install build-essential"
    HAS_ERROR=1
else
    MAKE_VERSION=$(make --version | head -1)
    echo "✅ make found: $MAKE_VERSION"
fi

# Summary
echo ""
echo "============================================"
if [ $HAS_ERROR -eq 0 ]; then
    echo "✅ Development environment is ready!"
    echo "   Run 'make build' to build the project"
else
    echo "❌ Please install missing dependencies"
    exit 1
fi
echo "============================================"
