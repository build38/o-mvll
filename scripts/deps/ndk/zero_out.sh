#!/bin/bash

BIN_DIR="$1"

KEEP_BINARIES=("clang-14" "clang" "clang++" "clang-cpp" "clang-cl" "clang-extdef-mapping" "clang-format"
               "clang-nvlink-wrapper" "clang-offload-bundler" "clang-offload-wrapper"
               "git-clang-format" "hmaptool" "llvm-config" "llvm-link" "llvm-lit"
               "llvm-tblgen" "FileCheck" "count" "not")

for file in "$BIN_DIR"/*; do
    if [[ ! "${KEEP_BINARIES[@]}" =~ "$(basename "$file")" ]]; then
        echo "Zeroing out $file"
        echo -n > "$file"
    fi
done
