#!/usr/bin/env bash
# This script is used to compile ndkr25c
set -ex

host=$(uname)

NDK_VERSION=26.1.10909125
MANIFEST_FILE="manifest_10552028.xml"

if [ "$host" == "Darwin" ]; then
    ndk_platform="darwin-x86_64"
else
    ndk_platform="linux-x86_64"
fi

mkdir android-llvm-toolchain-r26b && cd android-llvm-toolchain-r26b
repo init -u https://android.googlesource.com/platform/manifest -b llvm-toolchain

cp $ANDROID_HOME/ndk/$NDK_VERSION/toolchains/llvm/prebuilt/$ndk_platform/${MANIFEST_FILE} .repo/manifests/
repo init -m ${MANIFEST_FILE}
repo sync -c

python3 toolchain/llvm_android/build.py --skip-tests

export NDK_STAGE1=$(pwd)/out/stage1-install
export NDK_STAGE2=$(pwd)/out/stage2-install

# Cleanup stages folder before generating final package
zero_out.sh $NDK_STAGE1
zero_out.sh $NDK_STAGE2

# Generate final package
mkdir -p android-llvm-toolchain-r26b/out && cd android-llvm-toolchain-r26b/out
cp -r ${NDK_STAGE1} .
cp -r ${NDK_STAGE2} .

cd .. && tar czf out.tar.gz out && rm -rf out
cd .. && tar czf android-llvm-toolchain-r26b.tar.gz android-llvm-toolchain-r26b

# Clean up
rm -rf android-llvm-toolchain-r26b

