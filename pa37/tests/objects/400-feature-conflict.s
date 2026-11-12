.section .text.feature_stub,"",@
.globl feature_stub
feature_stub:
 .functype feature_stub () -> (i32)
 i32.const 17
 end_function
.section .custom_section.target_features,"",@
.int8 1
.int8 45
.int8 15
.ascii "reference-types"
