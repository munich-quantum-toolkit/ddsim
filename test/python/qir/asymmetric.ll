; Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
; Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
; SPDX-License-Identifier: MIT
;
; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

@qir.result_label_b_0 = internal constant [4 x i8] c"b_0\00"
@qir.result_label_b = internal constant [2 x i8] c"b\00"
@qir.result_label_a_1 = internal constant [4 x i8] c"a_1\00"
@qir.result_label_a_0 = internal constant [4 x i8] c"a_0\00"
@qir.result_label_a = internal constant [2 x i8] c"a\00"

define i64 @main() #0 {
  call void @__quantum__rt__initialize(ptr null)
  br label %1

1:                                                ; preds = %0
  call void @__quantum__qis__x__body(ptr null)
  call void @__quantum__qis__x__body(ptr inttoptr (i64 1 to ptr))
  br label %2

2:                                                ; preds = %1
  call void @__quantum__qis__mz__body(ptr null, ptr null)
  call void @__quantum__qis__mz__body(ptr inttoptr (i64 1 to ptr), ptr inttoptr (i64 1 to ptr))
  call void @__quantum__qis__mz__body(ptr inttoptr (i64 2 to ptr), ptr inttoptr (i64 2 to ptr))
  br label %3

3:                                                ; preds = %2
  call void @__quantum__rt__array_record_output(i64 2, ptr @qir.result_label_a)
  call void @__quantum__rt__result_record_output(ptr null, ptr @qir.result_label_a_0)
  call void @__quantum__rt__result_record_output(ptr inttoptr (i64 1 to ptr), ptr @qir.result_label_a_1)
  call void @__quantum__rt__array_record_output(i64 1, ptr @qir.result_label_b)
  call void @__quantum__rt__result_record_output(ptr inttoptr (i64 2 to ptr), ptr @qir.result_label_b_0)
  ret i64 0
}

declare void @__quantum__rt__initialize(ptr)

declare void @__quantum__qis__x__body(ptr)

declare void @__quantum__qis__mz__body(ptr, ptr writeonly) #1

declare void @__quantum__rt__array_record_output(i64, ptr)

declare void @__quantum__rt__result_record_output(ptr, ptr)

attributes #0 = { "entry_point" "output_labeling_schema"="labeled" "qir_profiles"="base_profile" "required_num_qubits"="3" "required_num_results"="3" }
attributes #1 = { "irreversible" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}

!0 = !{i32 1, !"qir_major_version", i32 2}
!1 = !{i32 7, !"qir_minor_version", i32 1}
!2 = !{i32 1, !"dynamic_qubit_management", i1 false}
!3 = !{i32 1, !"dynamic_result_management", i1 false}
!4 = !{i32 2, !"Debug Info Version", i32 3}
