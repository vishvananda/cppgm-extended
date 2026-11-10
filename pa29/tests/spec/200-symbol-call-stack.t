declare function @assembly_stack_callback() -> void [unwind=no]
assembly @call_void intel readwrite 13 0 7 1 {
  operand ptr 0 -1 1 0 0 { alternative 8 0 -1 {} }
  clobber rax
  clobber rcx
  clobber rdx
  clobber rsi
  clobber rdi
  clobber r8
  clobber r9
  clobber r10
  clobber r11
  clobber xmm0
  clobber xmm1
  clobber xmm2
  clobber xmm3
  clobber xmm4
  clobber xmm5
  clobber xmm6
  clobber xmm7
  clobber xmm8
  clobber xmm9
  clobber xmm10
  clobber xmm11
  clobber xmm12
  clobber xmm13
  clobber xmm14
  clobber xmm15
  text "call "
  use 0 1 0 0
}
function @direct_symbol_call() -> void {
  block ^entry:
    asm @call_void(@assembly_stack_callback)
    return void
}
