assembly @body intel none 1 0 0 0 {
 text ".text\n.globl module_answer\nmodule_answer:\nmov eax, 37\nret\n"
}
assembly_global @body()
declare function @module_answer() -> i32
function @main() -> i32 {
 block ^entry:
  %value = call i32 @module_answer()
  %result = binary sub i32 %value,37
  return i32 %result
}
