assembly @raw intel none 1 0 0 1 {
 operand i64 0 -1 1 0 0 { alternative 4 0 -1 {} }
 text "ret"
}
declare function @invalid(%arg : i64) -> i64
assembly_function @invalid {
 block ^entry:
  asm @raw(%arg)
  unreachable
}
