; Copyright (c) 2023 - 2026 Chair for Design Automation, TUM
; Copyright (c) 2025 - 2026 Munich Quantum Software Company GmbH
; SPDX-License-Identifier: MIT
;
; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

@qir.result_label_clean = internal constant [6 x i8] c"clean\00"
@qir.result_label_answer = internal constant [7 x i8] c"answer\00"
@qir.result_label_flag = internal constant [5 x i8] c"flag\00"

define i64 @main() #0 {
  call void @__quantum__rt__initialize(ptr null)
  %1 = alloca ptr, i64 1, align 8
  call void @__quantum__rt__result_array_allocate(i64 1, ptr %1, ptr null)
  %2 = alloca ptr, i64 1, align 8
  call void @__quantum__rt__result_array_allocate(i64 1, ptr %2, ptr null)
  %3 = alloca ptr, i64 1, align 8
  call void @__quantum__rt__result_array_allocate(i64 1, ptr %3, ptr null)
  br label %4

4:                                                ; preds = %0
  %5 = alloca ptr, i64 2, align 8
  call void @__quantum__rt__qubit_array_allocate(i64 2, ptr %5, ptr null)
  %6 = load ptr, ptr %5, align 8
  call void @__quantum__qis__h__body(ptr %6)
  %7 = load ptr, ptr %1, align 8
  call void @__quantum__qis__mz__body(ptr %6, ptr %7)
  %8 = call i1 @__quantum__rt__read_result(ptr %7)
  %9 = getelementptr ptr, ptr %5, i32 1
  %10 = load ptr, ptr %9, align 8
  br i1 %8, label %11, label %12

11:                                               ; preds = %4
  call void @__quantum__qis__x__body(ptr %10)
  br label %12

12:                                               ; preds = %11, %4
  call void @__quantum__qis__reset__body(ptr %6)
  %13 = load ptr, ptr %2, align 8
  call void @__quantum__qis__mz__body(ptr %10, ptr %13)
  %14 = load ptr, ptr %3, align 8
  call void @__quantum__qis__mz__body(ptr %6, ptr %14)
  call void @__quantum__rt__qubit_array_release(i64 2, ptr %5)
  br label %15

15:                                               ; preds = %12
  call void @__quantum__rt__result_array_record_output(i64 1, ptr %1, ptr @qir.result_label_flag)
  call void @__quantum__rt__result_array_record_output(i64 1, ptr %2, ptr @qir.result_label_answer)
  call void @__quantum__rt__result_array_record_output(i64 1, ptr %3, ptr @qir.result_label_clean)
  call void @__quantum__rt__result_array_release(i64 1, ptr %1)
  call void @__quantum__rt__result_array_release(i64 1, ptr %2)
  call void @__quantum__rt__result_array_release(i64 1, ptr %3)
  ret i64 0
}

declare void @__quantum__rt__initialize(ptr)

declare void @__quantum__rt__qubit_array_allocate(i64, ptr, ptr)

declare void @__quantum__qis__h__body(ptr)

declare void @__quantum__rt__result_array_allocate(i64, ptr, ptr)

declare void @__quantum__qis__mz__body(ptr, ptr writeonly) #1

declare i1 @__quantum__rt__read_result(ptr readonly)

declare void @__quantum__qis__x__body(ptr)

declare void @__quantum__qis__reset__body(ptr) #1

declare void @__quantum__rt__qubit_array_release(i64, ptr)

declare void @__quantum__rt__result_array_record_output(i64, ptr, ptr)

declare void @__quantum__rt__result_array_release(i64, ptr)

attributes #0 = { "entry_point" "output_labeling_schema"="labeled" "qir_profiles"="adaptive_profile" "required_num_qubits"="0" "required_num_results"="0" }
attributes #1 = { "irreversible" }

!llvm.module.flags = !{!0, !1, !2, !3, !4, !5, !6}

!0 = !{i32 1, !"qir_major_version", i32 2}
!1 = !{i32 7, !"qir_minor_version", i32 1}
!2 = !{i32 1, !"dynamic_qubit_management", i1 true}
!3 = !{i32 1, !"dynamic_result_management", i1 true}
!4 = !{i32 1, !"backwards_branching", i2 0}
!5 = !{i32 1, !"arrays", i1 true}
!6 = !{i32 2, !"Debug Info Version", i32 3}
