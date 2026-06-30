;
; This file is distributed under the Apache License v2.0. See LICENSE for details.
;

; REQUIRES: aarch64-registered-target && apple_abi

; Note: config_replace.py requests Local encoding for strings starting with
; "Hello", but the pass forces Global for CFString backings
;
;     RUN: env OMVLL_CONFIG=%S/config_replace.py clang++ -fpass-plugin=%libOMVLL \
;     RUN:         -target arm64-apple-ios26.0.0 -O1 -S -emit-llvm %s -o - | FileCheck %s
;
;     RUN: env OMVLL_CONFIG=%S/config_replace.py clang++ -fpass-plugin=%libOMVLL \
;     RUN:         -target arm64-apple-ios26.0.0 -O1 -c %s -o - | strings | FileCheck %s --check-prefix=STRINGS

; CHECK:      @[[BACKING:[a-zA-Z0-9_.$]+]] = {{.*}}global [23 x i8] c"

; The CFString struct is preserved
; CHECK:      { ptr @__CFConstantStringClassReference, i32 1992, ptr @[[BACKING]], i64 22 }, section "__DATA,__cfstring"

; CHECK:      @llvm.global_ctors = {{.*}}@__omvll_ctor_
; CHECK:      define {{.*}}@__omvll_ctor_

; The plaintext must not survive in the obfuscated output.
; CHECK-NOT:  Hello from a CFString!
; STRINGS-NOT: Hello from a CFString!

%struct.__NSConstantString_tag = type { ptr, i32, ptr, i64 }

@__CFConstantStringClassReference = external global [0 x i32]
@.str = private unnamed_addr constant [23 x i8] c"Hello from a CFString!\00", section "__TEXT,__cstring,cstring_literals", align 1
@_unnamed_cfstring_ = private global %struct.__NSConstantString_tag { ptr @__CFConstantStringClassReference, i32 1992, ptr @.str, i64 22 }, section "__DATA,__cfstring", align 8

declare void @CFShow(ptr noundef)

define void @use_cfstring() {
  call void @CFShow(ptr noundef @_unnamed_cfstring_)
  ret void
}
