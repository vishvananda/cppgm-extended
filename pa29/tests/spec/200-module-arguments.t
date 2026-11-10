assembly @module_body intel none 1 0 0 1 {
  operand i64 0 -1 1 0 0 { alternative 4 0 -1 {} }
  text ".text\n.globl module_constant\nmodule_constant:\nlea rax,[rdi + "
  use 0 1 0 0
  text "]\nret\n"
}
assembly_global @module_body(23)
