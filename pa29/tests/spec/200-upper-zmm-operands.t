assembly @upper_zmm_storage_add intel readwrite 1 0 0 3 {
 operand obj<64x1> 0 -1 1 1 0 { alternative 1 0 -1 { group (zmm29:0) } }
 operand obj<64x1> 1 -1 1 1 0 { alternative 1 0 -1 { group (zmm30:0) } }
 operand obj<64x1> -1 2 2 0 0 { alternative 1 1 -1 { group (zmm31:0) group (zmm29:0) } }
 text "vpaddd "
 use 2 0 0 0
 text ","
 use 0 0 0 0
 text ","
 use 1 0 0 0
}
function @upper_zmm_storage(%left : ptr, %right : ptr, %out : ptr) -> void {
 block ^entry:
  asm @upper_zmm_storage_add(%left,%right,%out)
  return void
}
assembly @upper_zmm_values_add intel readwrite 1 0 0 3 {
 operand obj<64x1> 0 -1 1 0 0 { alternative 1 0 -1 { group (zmm29:0) } }
 operand obj<64x1> 1 -1 1 0 0 { alternative 1 0 -1 { group (zmm30:0) } }
 operand obj<64x1> -1 2 2 0 0 { alternative 1 1 -1 { group (zmm31:0) group (zmm29:0) } }
 text "vpaddd "
 use 2 0 0 0
 text ","
 use 0 0 0 0
 text ","
 use 1 0 0 0
}
function @upper_zmm_values(%left : ptr, %right : ptr, %out : ptr) -> void {
 block ^entry:
  %left_bytes = load obj<64x1> %left
  %right_bytes = load obj<64x1> %right
  asm @upper_zmm_values_add(%left_bytes,%right_bytes,%out)
  return void
}
assembly @upper_zmm_alias intel write 1 0 0 3 {
 operand obj<64x1> -1 0 2 0 0 { alternative 1 1 -1 { group (zmm29:0) } }
 operand obj<64x1> -1 1 2 0 0 { alternative 1 1 -1 { group (zmm30:0) } }
 operand obj<64x1> 2 -1 1 1 0 { alternative 1 0 -1 { group (zmm31:0) } }
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
function @upper_zmm_alias_outputs(%input : ptr, %out : ptr) -> void {
 block ^entry:
  asm @upper_zmm_alias(%out,%out,%input)
  return void
}
