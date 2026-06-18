#!/usr/bin/sh

#
# This file is distributed under the Apache License v2.0. See LICENSE for details.
#

set -ex
mkdir -p /data && cd /data

cp /third-party/omvll-deps-ndk-*/android-llvm-toolchain-r29d.tar.gz .
cp /third-party/omvll-deps-ndk-*/Python-slim.tar.gz .
cp /third-party/omvll-deps-ndk-*/pybind11.tar.gz .
cp /third-party/omvll-deps-ndk-*/spdlog-1.10.0-Linux.tar.gz .

tar xzvf android-llvm-toolchain-r29d.tar.gz
tar xzvf Python-slim.tar.gz
tar xzvf pybind11.tar.gz
tar xzvf spdlog-1.10.0-Linux.tar.gz

# Android NDK is bootstrapped in a so-called 2-stage process. To avoid ABI incompatibilities,
# we build our plugin with the same toolchain used to build the NDK itself (stage-1). Then,
# we link the plugin against stage-2 build artifacts.
export NDK_STAGE1=$(pwd)/android-llvm-toolchain-r29d/out/stage1-install
export NDK_STAGE2=$(pwd)/android-llvm-toolchain-r29d/out/stage2

cp ${NDK_STAGE2}/bin/clang /test-deps/bin
cp ${NDK_STAGE2}/bin/clang++ /test-deps/bin

cd /o-mvll/src
mkdir -p o-mvll-build_ndk_r29d && cd o-mvll-build_ndk_r29d

# TODO: switch to release NDK
cmake -GNinja .. \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_COMPILER=${NDK_STAGE1}/bin/clang++ \
      -DCMAKE_C_COMPILER=${NDK_STAGE1}/bin/clang \
      -DCMAKE_CXX_FLAGS="-stdlib=libstdc++ -std=c++17" \
      -DPython3_ROOT_DIR=/data/Python-slim \
      -DPython3_LIBRARY=/data/Python-slim/lib/libpython3.10.a \
      -DPython3_INCLUDE_DIR=/data/Python-slim/include/python3.10 \
      -Dpybind11_DIR=/data/pybind11/share/cmake/pybind11 \
      -Dspdlog_DIR=/data/spdlog-1.10.0-Linux/lib/cmake/spdlog \
      -DLLVM_DIR=${NDK_STAGE2}/lib/cmake/llvm \
      -DLLVM_TOOLS_DIR=/test-deps \
      -DLLVM_EXTERNAL_LIT=/test-deps/bin/llvm-lit \
      -DOMVLL_ABI=CustomAndroid

export OMVLL_PYTHONPATH=/Python-3.10.7/Lib
ninja check

mv /o-mvll/src/o-mvll-build_ndk_r29d/libOMVLL.so /o-mvll/dist/omvll-ndk.so
