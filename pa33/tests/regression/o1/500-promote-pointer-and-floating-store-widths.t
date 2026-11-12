function @distance(%begin : ptr, %end : ptr) -> i32 [no_inline=yes, unwind=no] {
  slot $n : i32

  block ^entry:
    %d = binary sub ptr %end, %begin
    store typed i32 %d, $n
    %v = load typed i32 $n
    %r = binary add nowrap i32 %v, 1
    return i32 %r
}

function @address(%offset : i32) -> ptr [no_inline=yes, unwind=no] {
  slot $p : ptr

  block ^entry:
    %x = binary sub i32 %offset, 8
    store typed ptr %x, $p
    %v = load typed ptr $p
    return ptr %v
}

function @low_bits(%wide : i64) -> f32 [no_inline=yes, unwind=no] {
  slot $f : f32

  block ^entry:
    store typed i32 %wide, $f
    %v = load typed f32 $f
    return f32 %v
}

function @main() -> i32 [role=entry, unwind=no] {
  slot $buffer : obj<16x1>

  block ^entry:
    %p = addr $buffer
    %q = index i8 %p, 5
    %n = call i32 @distance(%p, %q)
    %a = call ptr @address(-16)
    %ai = copy i64 %a
    %f = call f32 @low_bits(5360320512)
    %bad_n = cmp ne i32 %n, 6
    %bad_a = cmp ne i64 %ai, -24
    %bad_f = cmp ne f32 %f, 1.0
    %bad_na = binary or i64 %bad_n, %bad_a
    %bad = binary or i64 %bad_na, %bad_f
    %status = convert trunc i32 i64 %bad
    return i32 %status
}
