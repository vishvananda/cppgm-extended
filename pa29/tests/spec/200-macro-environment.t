global @macro_value : i64 = 41
assembly @first intel none 1 0 0 1 {
 operand ptr 0 -1 1 0 0 { alternative 8 0 -1 {} }
 text ".macro read_value destination:req\nmov \\destination,qword ptr [rip+"
 use 0 1 0 0
 text "]\nadd \\destination,2\n.endm\n"
}
assembly @second intel none 1 0 0 0 {
 text ".purgem read_value\n.macro read_value destination:req\nmov \\destination,73\n.endm\n"
}
assembly @read_first intel none 1 0 0 1 environment 1 {
 operand i64 -1 0 2 0 0 { alternative 1 0 -1 { group (rax) } }
 text "read_value "
 use 0 0 0 0
}
assembly @read_second intel none 1 0 0 1 environment 2 {
 operand i64 -1 0 2 0 0 { alternative 1 0 -1 { group (rax) } }
 text "read_value "
 use 0 0 0 0
}
assembly @raw_first intel none 1 0 0 0 environment 1 {
 text "read_value rax\nret\n"
}
assembly_global @first(@macro_value)
assembly_global @second()
function @macro_earlier() -> i64 {
 slot $value : i64
 block ^entry:
  %out = addr $value
  asm @read_first(%out)
  %value = load i64 $value
  return i64 %value
}
function @macro_later() -> i64 {
 slot $value : i64
 block ^entry:
  %out = addr $value
  asm @read_second(%out)
  %value = load i64 $value
  return i64 %value
}
declare function @macro_raw_earlier() -> i64
assembly_function @macro_raw_earlier {
 block ^entry:
  asm @raw_first()
  unreachable
}
