//
// This file is distributed under the Apache License v2.0. See LICENSE for
// details.
//

// REQUIRES: aarch64-registered-target && android_abi

// RUN: env OMVLL_CONFIG=%S/config_all.py clang -target aarch64-linux-android -fpass-plugin=%libOMVLL -O1 -fno-verbose-asm -S %s -o - | FileCheck --check-prefix=FLAT-ANDROID %s

// Check for jump table targets setup at the beginning of the function.

// FLAT-ANDROID-LABEL:    check_password:
// FLAT-ANDROID:	mov	w16, #42
// FLAT-ANDROID-NEXT:	movk	w17, #41630, lsl #16
// FLAT-ANDROID-NEXT:	movk	w2, #36449, lsl #16
// FLAT-ANDROID-NEXT:	movk	w3, #59726, lsl #16
// FLAT-ANDROID-NEXT:	movk	w4, #30851, lsl #16
// FLAT-ANDROID-NEXT:	movk	w5, #1377, lsl #16
// FLAT-ANDROID-NEXT:	movk	w6, #22996, lsl #16
// FLAT-ANDROID-NEXT:	movk	w7, #4877, lsl #16
// FLAT-ANDROID-NEXT:	movk	w19, #37345, lsl #16
// FLAT-ANDROID-NEXT:	movk	w20, #5862, lsl #16
// FLAT-ANDROID-NEXT:	movk	w21, #22996, lsl #16
// FLAT-ANDROID-NEXT:	movk	w22, #59726, lsl #16
// FLAT-ANDROID-NEXT:	movk	w23, #1377, lsl #16
// FLAT-ANDROID-NEXT:	movk	w24, #22996, lsl #16
// FLAT-ANDROID-NEXT:	movk	w25, #30851, lsl #16
// FLAT-ANDROID-NEXT:	movk	w26, #4877, lsl #16
// FLAT-ANDROID-NEXT:	stur	w11, [x29, #-4]
// FLAT-ANDROID-NEXT:	b	.LBB0_4


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
