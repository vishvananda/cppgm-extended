declare function @module_callback(%value : i64) -> i64
assembly @module_address intel none 1 0 0 1 {
  operand ptr 0 -1 1 0 0 { alternative 8 0 -1 {} }
  text ".data\n.balign 8\n.globl module_function_pointer\nmodule_function_pointer:\n.quad "
  use 0 1 0 0
  text "\n"
}
assembly_global @module_address(@module_callback)
