; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=amdgcn-amd-amdhsa --global-isel -verify-machineinstrs < %s | FileCheck --check-prefix=PREGFX9 %s
; RUN: llc -mtriple=amdgcn-amd-amdhsa --global-isel -mcpu=hawaii -verify-machineinstrs < %s | FileCheck --check-prefix=PREGFX9 %s
; RUN: llc -mtriple=amdgcn-amd-amdhsa --global-isel -mcpu=fiji -verify-machineinstrs < %s | FileCheck --check-prefix=PREGFX9 %s
; RUN: llc -mtriple=amdgcn-amd-amdhsa --global-isel -mcpu=gfx90a -verify-machineinstrs < %s | FileCheck --check-prefix=PREGFX9 %s
; RUN: llc -mtriple=amdgcn-amd-amdhsa --global-isel -mcpu=gfx1030 -verify-machineinstrs < %s | FileCheck --check-prefix=GFX10PLUS %s
; RUN: llc -mtriple=amdgcn-amd-amdhsa --global-isel -mcpu=gfx1100 -verify-machineinstrs < %s | FileCheck --check-prefix=GFX10PLUS %s

define i32 @check_v_bfe(i16 %a) {
; PREGFX9-LABEL: check_v_bfe:
; PREGFX9:       ; %bb.0: ; %entry
; PREGFX9-NEXT:    s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
; PREGFX9-NEXT:    v_bfe_i32 v0, v0, 0, 16
; PREGFX9-NEXT:    s_setpc_b64 s[30:31]
;
; GFX10PLUS-LABEL: check_v_bfe:
; GFX10PLUS:       ; %bb.0: ; %entry
; GFX10PLUS-NEXT:    s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
; GFX10PLUS-NEXT:    v_bfe_i32 v0, v0, 0, 16
; GFX10PLUS-NEXT:    s_setpc_b64 s[30:31]
entry:
  %res = sext i16 %a to i32
  ret i32 %res
}