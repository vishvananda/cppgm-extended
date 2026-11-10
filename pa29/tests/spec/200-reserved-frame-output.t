assembly @carry intel none 1 0 1 0 { text "stc" }
assembly @reserved intel none 1 1 0 2 {
  operand i64 -1 0 2 0 0 { alternative 1 0 -1 { group (r12:0) } }
  operand i32 -1 1 10 0 2 {}
  text "mov "
  use 0 0 0 0
  text ", 123"
}
function @reserved_output(%out : ptr) -> void {
  slot $wide : obj<128x64>
  block ^entry:
    %p = addr $wide
    %q = index i64 %p, 1
    %guard = index i64 %p, 15
    store i64 97, %guard
    asm @carry()
    asm @reserved(%p, %q)
    %value = load i64 %p
    %flag = load i32 %q
    %kept = load i64 %guard
    store i64 %value, %out
    %a = index i64 %out, 1
    %wideflag = convert zext i64 i32 %flag
    store i64 %wideflag, %a
    %b = index i64 %out, 2
    store i64 %kept, %b
    return void
}
