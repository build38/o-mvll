//
// This file is distributed under the Apache License v2.0. See LICENSE for
// details.
//

// REQUIRES: aarch64-registered-target && android_abi

// RUN:                                        clang -target aarch64-linux-android                         -O1 -fno-verbose-asm -S %s -o - | FileCheck --check-prefix=R0 %s
// RUN: env OMVLL_CONFIG=%S/config_rounds_0.py clang -target aarch64-linux-android -fpass-plugin=%libOMVLL -O1 -fno-verbose-asm -S %s -o - | FileCheck --check-prefix=R0 %s
// RUN: env OMVLL_CONFIG=%S/config_rounds_1.py clang -target aarch64-linux-android -fpass-plugin=%libOMVLL -O1 -fno-verbose-asm -S %s -o - | FileCheck --check-prefix=R1 %s
// RUN: env OMVLL_CONFIG=%S/config_rounds_2.py clang -target aarch64-linux-android -fpass-plugin=%libOMVLL -O1 -fno-verbose-asm -S %s -o - | FileCheck --check-prefix=R2 %s
// RUN: env OMVLL_CONFIG=%S/config_rounds_3.py clang -target aarch64-linux-android -fpass-plugin=%libOMVLL -O1 -fno-verbose-asm -S %s -o - | FileCheck --check-prefix=R3 %s

// R0-LABEL: memcpy_xor:
// R0:       .LBB0_2:
// R0:           ldrb	w11, [x1], #1
// R0:           subs	x8, x8, #1
// R0:           eor	w11, w11, w9
// R0:           strb	w11, [x10], #1
// R0:           b.ne	.LBB0_2

// R1-LABEL: memcpy_xor:
// R1:       .LBB0_2:
// R1-NEXT:   	ldrb	w11, [x1], #1
// R1-NEXT:   	subs	x8, x8, #1
// R1-NEXT:   	eor	w11, w11, w9
// R1-NEXT:   	strb	w11, [x10], #1
// R1-NEXT:   	b.ne	.LBB0_2

// R2-LABEL: memcpy_xor:
// R2:       .LBB0_2:
// R2-NEXT:	ldrb	w12, [x1], #1
// R2-NEXT:	subs	x8, x8, #1
// R2-NEXT:	orn	w13, w9, w12
// R2-NEXT:	add	w13, w12, w13
// R2-NEXT:	and	w12, w12, w10
// R2-NEXT:	add	w13, w13, #36
// R2-NEXT:	neg	w12, w12
// R2-NEXT:	eor	w14, w13, w12
// R2-NEXT:	and	w12, w13, w12
// R2-NEXT:	add	w12, w14, w12, lsl #1
// R2-NEXT:	strb	w12, [x11], #1
// R2-NEXT:	b.ne	.LBB0_2

// R3-LABEL: memcpy_xor:
// R3:       .LBB0_2:
// R3-NEXT: 	ldrb	w12, [x1, w8, uxtw]
// R3-NEXT: 	mvn	w15, w8
// R3-NEXT: 	and	w17, w8, #0x1
// R3-NEXT: 	bic	w3, w11, w8, lsl #1
// R3-NEXT: 	orr	w15, w15, #0x1
// R3-NEXT: 	add	w17, w8, w17
// R3-NEXT: 	add	w13, w12, #36
// R3-NEXT: 	and	w14, w12, w9
// R3-NEXT: 	sub	w12, w10, w12
// R3-NEXT: 	sub	w16, w14, w13
// R3-NEXT: 	sub	w14, w13, w14
// R3-NEXT: 	sub	w12, w12, w16
// R3-NEXT: 	and	w4, w12, w16
// R3-NEXT: 	orr	w12, w12, w16
// R3-NEXT: 	add	w16, w8, #1
// R3-NEXT: 	add	w12, w14, w12
// R3-NEXT: 	add	w14, w17, w3
// R3-NEXT: 	eor	w17, w16, w15
// R3-NEXT: 	add	w12, w4, w12
// R3-NEXT: 	and	w15, w16, w15
// R3-NEXT: 	add	w14, w14, w17
// R3-NEXT: 	add	w12, w13, w12, lsl #1
// R3-NEXT: 	add	w13, w14, w15, lsl #1
// R3-NEXT: 	sub	w12, w12, #1
// R3-NEXT: 	strb	w12, [x0, w8, uxtw]
// R3-NEXT: 	sub	w8, w13, #1
// R3-NEXT: 	cmp	w8, w2
// R3-NEXT: 	b.lo	.LBB0_2


void memcpy_xor(char *dst, const char *src, unsigned len) {
  for (unsigned i = 0; i < len; i += 1) {
    dst[i] = src[i] ^ 35;
  }
  dst[len] = '\0';
}
