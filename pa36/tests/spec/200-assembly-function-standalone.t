assembly @raw_entry intel none 1 0 1 0 {
 text "xor eax,eax; ret"
}
declare function @main() -> i32
assembly_function @main initial {
 block ^entry:
  asm @raw_entry()
  unreachable
}
