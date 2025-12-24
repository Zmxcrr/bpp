#!/bin/bash

# build.sh - Build script for C++23 interpreter project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_note() {
    echo -e "${BLUE}[NOTE]${NC} $1"
}

# Check compiler versions for C++23 support
check_compiler() {
    if command -v g++ >/dev/null 2>&1; then
        GCC_VERSION=$(g++ -dumpversion)
        print_note "Found GCC version: $GCC_VERSION"

        # Check if GCC version is >= 11
        if ! g++ -std=c++23 -x c++ -c /dev/null -o /dev/null 2>/dev/null; then
            print_error "GCC does not support C++23. Please upgrade to GCC 11 or later."
            print_note "You can also try using Clang 12+ instead."
            exit 1
        fi
    elif command -v clang++ >/dev/null 2>&1; then
        CLANG_VERSION=$(clang++ --version | head -n1)
        print_note "Found Clang: $CLANG_VERSION"

        if ! clang++ -std=c++23 -x c++ -c /dev/null -o /dev/null 2>/dev/null; then
            print_error "Clang does not support C++23. Please upgrade to Clang 12 or later."
            exit 1
        fi
    else
        print_error "No suitable C++ compiler found. Please install GCC 11+ or Clang 12+."
        exit 1
    fi

    print_status "C++23 compiler support verified!"
}

# Default values
BUILD_TYPE="Release"
BUILD_EXAMPLES=ON
BUILD_TESTS=OFF
CLEAN=false
INSTALL=false
VERBOSE=false
ENABLE_GC=ON
ENABLE_SIMPLE_JIT=ON

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --gc)
            ENABLE_GC=ON
            shift
            ;;
        --no-gc)
            ENABLE_GC=OFF
            shift
            ;;
        --jit)
            ENABLE_SIMPLE_JIT=ON
            shift
            ;;
        --no-jit)
            ENABLE_SIMPLE_JIT=OFF
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --examples)
            BUILD_EXAMPLES=ON
            shift
            ;;
        --no-examples)
            BUILD_EXAMPLES=OFF
            shift
            ;;
        --tests)
            BUILD_TESTS=ON
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --install)
            INSTALL=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug         Build in Debug mode (default: Release)"
            echo "  --examples      Build examples (default: ON)"
            echo "  --no-examples   Don't build examples"
            echo "  --tests         Build tests (default: OFF)"
            echo "  --clean         Clean build directory first"
            echo "  --install       Install after building"
            echo "  --verbose       Verbose build output"
            echo "  --help, -h      Show this help message"
            echo ""
            echo "C++23 Requirements:"
            echo "  - GCC 11+ or Clang 12+ or MSVC 2022+"
            echo "  - CMake 3.20+"
            echo "  --gc            Enable GC (default: ON)"
            echo "  --no-gc         Disable GC"
            echo "  --jit           Enable SimpleJIT (default: ON)"
            echo "  --no-jit        Disable SimpleJIT"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

print_status "C++23 Interpreter Build Script"
print_status "=============================="

# Check compiler support
check_compiler

# Check if we're in the right directory
if [[ ! -f "CMakeLists.txt" ]]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Clean if requested
if [[ "$CLEAN" == true ]]; then
    print_status "Cleaning build directory..."
    rm -rf build
fi

# Create build directory
mkdir -p build
cd build

print_status "Configuring C++23 project..."
print_status "Build type: $BUILD_TYPE"
print_status "Examples: $BUILD_EXAMPLES"

# Configure with CMake
CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DBUILD_EXAMPLES=$BUILD_EXAMPLES"
    "-DCMAKE_CXX_STANDARD=23"
    "-DCMAKE_CXX_STANDARD_REQUIRED=ON"
    "-DENABLE_GC=$ENABLE_GC"
    "-DENABLE_SIMPLE_JIT=$ENABLE_SIMPLE_JIT"
)

if [[ "$VERBOSE" == true ]]; then
    CMAKE_ARGS+=("-DCMAKE_VERBOSE_MAKEFILE=ON")
fi

cmake .. "${CMAKE_ARGS[@]}"

if [[ $? -ne 0 ]]; then
    print_error "CMake configuration failed!"
    exit 1
fi

print_status "Building project with C++23..."

# Build
if [[ "$VERBOSE" == true ]]; then
    cmake --build . --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) --verbose
else
    cmake --build . --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
fi

if [[ $? -ne 0 ]]; then
    print_error "Build failed!"
    exit 1
fi

print_status "Build completed successfully!"

# Install if requested
if [[ "$INSTALL" == true ]]; then
    print_status "Installing..."
    cmake --install .
fi

print_status "All done!"
echo ""
print_status "Build artifacts:"
echo "  Library: build/lib/"
if [[ "$BUILD_EXAMPLES" == "ON" ]]; then
    echo "  Examples: build/examples/"
fi
echo ""
print_note "C++23 features enabled: std::ranges, modules (if supported), and more!"
