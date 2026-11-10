assembly @copy_dwords intel readwrite 1 32 0 6 {
  operand ptr 0 3 3 0 0 { alternative 1 0 -1 { group (rdi) } }
  operand ptr 1 4 3 0 0 { alternative 1 0 -1 { group (rsi) } }
  operand i64 2 5 3 0 0 { alternative 1 0 -1 { group (rcx) } }
  text "rep movsd"
}
function @direct_copy_dwords(%destination : ptr, %source : ptr, %count : i64) -> void {
  slot $d : ptr
  slot $s : ptr
  slot $c : i64
  block ^entry:
    %d = addr $d
    %s = addr $s
    %c = addr $c
    asm @copy_dwords(%destination, %source, %count, %d, %s, %c)
    return void
}
