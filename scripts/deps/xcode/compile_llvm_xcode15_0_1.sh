#!/usr/bin/env bash
# This script is used to compile xcode llvm
set -e
LLVM_TARGET="AArch64;X86"

git clone -j8 --branch swift/release/5.9.0 --single-branch --depth 1 \
          https://github.com/apple/llvm-project.git

echo "LLVM XCODE Version: $(git --git-dir=llvm-project/.git describe --dirty)"

LLVM_ROOT=$(pwd)/llvm-project
BUILD_ROOT=$(pwd)/build
cmake -GNinja -S ${LLVM_ROOT}/llvm \
      -B ${BUILD_ROOT}                                                                  \
      -DCMAKE_BUILD_TYPE=Release               \
      -DCMAKE_OSX_ARCHITECTURES="arm64" \
      -DCMAKE_CXX_FLAGS="-stdlib=libc++" \
      -DLLVM_TARGET_ARCH=${LLVM_TARGET}        \
      -DLLVM_TARGETS_TO_BUILD=${LLVM_TARGET}   \
      -DLLVM_ENABLE_PROJECTS="clang;llvm"  \
      -DCMAKE_OSX_DEPLOYMENT_TARGET="13.2"

ninja -C ${BUILD_ROOT} package
cp ${BUILD_ROOT}/LLVM-16.0.0git-Darwin.tar.gz ./omvll-deps/LLVM-16.0.0git-arm64-Darwin.tar.gz
rm -rf ${BUILD_ROOT}

cmake -GNinja -S ${LLVM_ROOT}/llvm \
      -B ${BUILD_ROOT}                                                                  \
      -DCMAKE_BUILD_TYPE=Release               \
      -DCMAKE_OSX_ARCHITECTURES="x86_64" \
      -DCMAKE_CXX_FLAGS="-stdlib=libc++" \
      -DLLVM_TARGET_ARCH=${LLVM_TARGET}        \
      -DLLVM_TARGETS_TO_BUILD=${LLVM_TARGET}   \
      -DLLVM_ENABLE_PROJECTS="clang;llvm"  \
      -DCMAKE_OSX_DEPLOYMENT_TARGET="13.2"

ninja -C ${BUILD_ROOT} package
cp ${BUILD_ROOT}/LLVM-16.0.0git-Darwin.tar.gz ./omvll-deps/LLVM-16.0.0git-x86_64-Darwin.tar.gz
rm -rf ${BUILD_ROOT} && rm -rf llvm-project
