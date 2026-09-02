//
// This file is distributed under the Apache License v2.0. See LICENSE for
// details.
//

// REQUIRES: aarch64-registered-target && android_abi

// RUN: env OMVLL_CONFIG=%S/config_all.py clang -target aarch64-linux-android -fpass-plugin=%libOMVLL -O1 -fno-verbose-asm -S %s -o - | FileCheck --check-prefix=FLAT-ANDROID %s

// Check for the dispatcher's default case, which only the flattening pass
// emits. The state values themselves depend on the RNG seed.

// FLAT-ANDROID-LABEL:    check_password:
// FLAT-ANDROID:            ldr	x1, #-8
// FLAT-ANDROID:            blr	x1
// FLAT-ANDROID:            mov	x0, x1

int check_password(const char *passwd, unsigned len) {
  if (len != 5) {
    return 0;
  }
  if (passwd[0] == 'O') {
    if (passwd[1] == 'M') {
      if (passwd[2] == 'V') {
        if (passwd[3] == 'L') {
          if (passwd[4] == 'L') {
            return 1;
          }
        }
      }
    }
  }
  return 0;
}
