assembly @mask_packet_8 intel write 1 0 0 2 {
 operand i8 0 -1 1 0 0 { alternative 1 0 -1 { group (k0:0) } }
 operand i8 -1 1 2 0 0 { alternative 1 1 -1 { group (k7:0) group (k6:0) } }
 clobber k6
 text "kmovb "
 use 1 0 0 0
 text ","
 use 0 0 0 0
}
function @mask_lowir_8(%value : i8, %out : ptr) -> void {
 block ^entry:
  asm @mask_packet_8(%value,%out)
  return void
}
assembly @mask_packet_16 intel write 1 0 0 2 {
 operand i16 0 -1 1 0 0 { alternative 1 0 -1 { group (k0:0) } }
 operand i16 -1 1 2 0 0 { alternative 1 1 -1 { group (k7:0) group (k6:0) } }
 clobber k6
 text "kmovw "
 use 1 0 0 0
 text ","
 use 0 0 0 0
}
function @mask_lowir_16(%value : i16, %out : ptr) -> void {
 block ^entry:
  asm @mask_packet_16(%value,%out)
  return void
}
assembly @mask_packet_32 intel write 1 0 0 2 {
 operand i32 0 -1 1 0 0 { alternative 1 0 -1 { group (k0:0) } }
 operand i32 -1 1 2 0 0 { alternative 1 1 -1 { group (k7:0) group (k6:0) } }
 clobber k6
 text "kmovd "
 use 1 0 0 0
 text ","
 use 0 0 0 0
}
function @mask_lowir_32(%value : i32, %out : ptr) -> void {
 block ^entry:
  asm @mask_packet_32(%value,%out)
  return void
}
assembly @mask_packet_64 intel write 1 0 0 2 {
 operand i64 0 -1 1 0 0 { alternative 1 0 -1 { group (k0:0) } }
 operand i64 -1 1 2 0 0 { alternative 1 1 -1 { group (k7:0) group (k6:0) } }
 clobber k6
 text "kmovq "
 use 1 0 0 0
 text ","
 use 0 0 0 0
}
function @mask_lowir_64(%value : i64, %out : ptr) -> void {
 block ^entry:
  asm @mask_packet_64(%value,%out)
  return void
}
