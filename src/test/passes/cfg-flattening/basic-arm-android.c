//
// This file is distributed under the Apache License v2.0. See LICENSE for
// details.
//

// REQUIRES: arm-registered-target && android_abi

// RUN:                                   clang -target arm-linux-android                         -O1 -fno-verbose-asm -S %s -o - | FileCheck --check-prefix=NO-FLAT %s
// RUN: env OMVLL_CONFIG=%S/config_all.py clang -target arm-linux-android -fpass-plugin=%libOMVLL -O1 -fno-verbose-asm -S %s -o - | FileCheck --check-prefix=FLAT-ARM %s
// RUN: env OMVLL_CONFIG=%S/config_all.py clang -target armv7-none-linux-androideabi -fpass-plugin=%libOMVLL -O1 -fno-verbose-asm -S %s -o - | FileCheck --check-prefix=FLAT-THUMB %s

// Check for jump table targets setup at the beginning of the function.

// NO-FLAT-LABEL: check_password:
// NO-FLAT:         mov	r2, #0
// NO-FLAT-NEXT:    cmp	r1, #5
// NO-FLAT-NEXT:    bne	.LBB0_3
// NO-FLAT-NEXT:    ldrb	r1, [r0]
// NO-FLAT-NEXT:    cmp	r1, #79
// NO-FLAT-NEXT:    ldrbeq	r1, [r0, #1]
// NO-FLAT-NEXT:    cmpeq	r1, #77
// NO-FLAT-NEXT:    beq	.LBB0_4

// Check for the dispatcher's default case, which only the flattening pass
// emits. The state values themselves depend on the RNG seed.

// FLAT-ARM-LABEL:   check_password:
// FLAT-ARM:           ldr	r1, [pc, #-4]
// FLAT-ARM:           bx	r1
// FLAT-ARM:           mov	r1, r2

// FLAT-THUMB-LABEL: check_password:
// FLAT-THUMB:         ldr	r1, [pc, #-4]
// FLAT-THUMB:         bx	r1
// FLAT-THUMB:         mov	r1, r2

int check_password(const char* passwd, unsigned len) {
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
