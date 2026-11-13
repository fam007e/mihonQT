#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "=== Running MihonQT Unit Test Suite ==="
cd "${ROOT_DIR}"

if [ ! -d "build" ]; then
    echo "Configuring build directory..."
    cmake -B build -S . -DBUILD_TESTS=ON
fi

echo "Building test suite..."
cmake --build build -j"$(nproc)"

echo "Executing tests with CTest..."
ctest --test-dir build --output-on-failure
