assembly @wide_storage_add intel readwrite 1 0 0 3 {
 operand obj<32x1> 0 -1 1 1 0 { alternative 1 0 -1 { group (ymm14:0) } }
 operand obj<32x1> 1 -1 1 1 0 { alternative 1 0 -1 { group (ymm15:0) } }
 operand obj<32x1> -1 2 2 0 0 { alternative 1 1 -1 { group (ymm0:0) group (ymm14:0) } }
 text "vpaddd "
 use 2 0 0 0
 text ","
 use 0 0 0 0
 text ","
 use 1 0 0 0
}
function @wide_storage(%left : ptr, %right : ptr, %out : ptr) -> void {
 block ^entry:
  asm @wide_storage_add(%left,%right,%out)
  return void
}
assembly @wide_values_add intel readwrite 1 0 0 3 {
 operand obj<32x1> 0 -1 1 0 0 { alternative 1 0 -1 { group (ymm14:0) } }
 operand obj<32x1> 1 -1 1 0 0 { alternative 1 0 -1 { group (ymm15:0) } }
 operand obj<32x1> -1 2 2 0 0 { alternative 1 1 -1 { group (ymm0:0) group (ymm14:0) } }
 text "vpaddd "
 use 2 0 0 0
 text ","
 use 0 0 0 0
 text ","
 use 1 0 0 0
}
function @wide_values(%left : ptr, %right : ptr, %out : ptr) -> void {
 block ^entry:
  %left_bytes = load obj<32x1> %left
  %right_bytes = load obj<32x1> %right
  asm @wide_values_add(%left_bytes,%right_bytes,%out)
  return void
}
assembly @wide_alias intel write 1 0 0 3 {
 operand obj<32x1> -1 0 2 0 0 { alternative 1 1 -1 { group (ymm14:0) } }
 operand obj<32x1> -1 1 2 0 0 { alternative 1 1 -1 { group (ymm15:0) } }
 operand obj<32x1> 2 -1 1 1 0 { alternative 1 0 -1 { group (ymm0:0) } }
 text "vpslld "
 use 0 0 0 0
 text ","
 use 2 0 0 0
 text ",1\nvpslld "
 use 1 0 0 0
 text ","
 use 2 0 0 0
 text ",2"
}
function @wide_alias_outputs(%input : ptr, %out : ptr) -> void {
 block ^entry:
  asm @wide_alias(%out,%out,%input)
  return void
}
