assembly @edge intel none 1 0 0 3 {
  operand i32 0 -1 1 0 0 { alternative 16 0 -1 {} }
  operand i32 1 -1 1 0 0 { alternative 16 0 -1 {} }
  operand i32 -1 2 66 0 0 {}
  clobber rax
  clobber rbx
  clobber rcx
  clobber rdx
  clobber rsi
  clobber rdi
  clobber r8
  clobber r9
  clobber r10
  clobber r11
  clobber r12
  clobber r13
  clobber r14
  clobber r15
  text "jmp "
  use 0 3 0 0
}
assembly @fallthrough intel none 1 0 0 2 {
  operand i32 0 -1 1 0 0 { alternative 16 0 -1 {} }
  operand i32 -1 1 66 0 0 {}
  text "nop"
}
function @direct_exit_tags(%out : ptr) -> void {
  slot $tag : i32
  block ^entry:
    %p = addr $tag
    asm @edge(17, 23, %p)
    %a = load i32 $tag
    store i32 %a, %out
    asm @edge(41, 59, %p)
    %b = load i32 $tag
    %q = index i32 %out, 1
    store i32 %b, %q
    asm @fallthrough(73, %p)
    %c = load i32 $tag
    %r = index i32 %out, 2
    store i32 %c, %r
    return void
}
