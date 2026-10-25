; Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
; Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
; SPDX-License-Identifier: MIT
;
; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

define i64 @main() #0 {
  call void @__quantum__rt__initialize(ptr null)
  br label %1

1:                                                ; preds = %0
  call void @__quantum__qis__h__body(ptr null)
  call void @__quantum__qis__x__body(ptr inttoptr (i64 1 to ptr))
  call void @__quantum__qis__cs__adj(ptr inttoptr (i64 1 to ptr), ptr null)
  call void @__quantum__qis__gphase__body(double 3.700000e-01)
  br label %2

2:                                                ; preds = %1
  br label %3

3:                                                ; preds = %2
  ret i64 0
}

declare void @__quantum__rt__initialize(ptr)

declare void @__quantum__qis__h__body(ptr)

declare void @__quantum__qis__x__body(ptr)

declare void @__quantum__qis__cs__adj(ptr, ptr)

declare void @__quantum__qis__gphase__body(double)

attributes #0 = { "entry_point" "output_labeling_schema"="labeled" "qir_profiles"="base_profile" "required_num_qubits"="2" "required_num_results"="0" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}

!0 = !{i32 1, !"qir_major_version", i32 2}
!1 = !{i32 7, !"qir_minor_version", i32 1}
!2 = !{i32 1, !"dynamic_qubit_management", i1 false}
!3 = !{i32 1, !"dynamic_result_management", i1 false}
!4 = !{i32 2, !"Debug Info Version", i32 3}
