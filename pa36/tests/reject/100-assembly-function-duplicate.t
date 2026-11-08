assembly @raw intel none 1 0 0 0 { text "ret" }
declare function @invalid() -> void
assembly_function @invalid {
 block ^entry:
  asm @raw()
  unreachable
}
assembly_function @invalid {
 block ^entry:
  asm @raw()
  unreachable
}
