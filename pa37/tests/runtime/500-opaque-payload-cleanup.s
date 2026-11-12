.text
.tagtype __cppgm_opaque_exception i64, i64
.weak __cppgm_opaque_exception
__cppgm_opaque_exception:

.section .text.raise_foreign,"",@
.globl raise_foreign
raise_foreign:
 .functype raise_foreign (i64) -> ()
 local.get 0
 i64.const 0
 throw __cppgm_opaque_exception
 end_function
