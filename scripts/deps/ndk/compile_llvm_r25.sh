#!/usr/bin/env bash
# This script is used to compile ndkr25c
set -ex

mkdir android-llvm-toolchain-r25c && cd android-llvm-toolchain-r25c
repo init -u https://android.googlesource.com/platform/manifest -b llvm-toolchain

#TODO this manifest name depends on compilation platform windows, linux, or macos
cp $ANDROID_HOME/ndk/25.1.8937393/toolchains/llvm/prebuilt/linux-x86_64/manifest_9352603.xml .repo/manifests/
repo init -m manifest_9352603.xml
repo sync -c

python3 toolchain/llvm_android/build.py --skip-tests

export NDK_STAGE1=$(pwd)/out/stage1-install
export NDK_STAGE2=$(pwd)/out/stage2-install

mkdir -p android-llvm-toolchain-r25c/out && cd android-llvm-toolchain-r25c/out
cp -r ${NDK_STAGE1} .
cp -r ${NDK_STAGE2} .
cd .. && tar czf out.tar.gz out && rm -rf out
cd .. && tar czf ../omvll-deps/android-llvm-toolchain-r25c.tar.gz android-llvm-toolchain-r25c


rm -rf android-llvm-toolchain-r25c


#in docker SDKManager needs to be present, repo command installed, rsync needs to be installed
