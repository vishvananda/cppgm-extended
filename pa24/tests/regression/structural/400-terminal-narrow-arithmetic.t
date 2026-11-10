function @wrap_unsigned(%value : u8) -> u8 [unwind=no] {
  block ^entry:
    %result = binary add u8 %value, 1
    return u8 %result
}
function @wrap_signed(%value : i8) -> i8 [unwind=no] {
  block ^entry:
    %result = binary sub i8 %value, 1
    return i8 %result
}
function @store_word(%value : u16, %destination : ptr) -> void [unwind=no] {
  block ^entry:
    %result = binary add u16 %value, 1
    store volatile u16 %result, %destination
    return void
}
function @widen_unsigned(%value : u8) -> i64 [unwind=no] {
  block ^entry:
    %result = binary add u8 %value, 1
    %wide = convert zext i64 u8 %result
    return i64 %wide
}
function @widen_signed(%value : i8) -> i64 [unwind=no] {
  block ^entry:
    %result = binary sub i8 %value, 1
    %wide = convert sext i64 i8 %result
    return i64 %wide
}
function @wrap_bool(%value : i1) -> i1 [unwind=no] {
  block ^entry:
    %result = binary add i1 %value, 1
    return i1 %result
}
function @main() -> i32 [role=entry, unwind=no] {
  slot $word : u16
  block ^entry:
    %unsigned = call u8 @wrap_unsigned(255)
    %bad_unsigned = cmp ne u8 %unsigned, 0
    %signed = call i8 @wrap_signed(-128)
    %bad_signed = cmp ne i8 %signed, 127
    %destination = addr $word
    call void @store_word(65535, %destination)
    %word = load volatile u16 $word
    %bad_word = cmp ne u16 %word, 0
    %wide_unsigned = call i64 @widen_unsigned(255)
    %bad_wide_unsigned = cmp ne i64 %wide_unsigned, 0
    %wide_signed = call i64 @widen_signed(-128)
    %bad_wide_signed = cmp ne i64 %wide_signed, 127
    %false = call i1 @wrap_bool(1)
    %true = call i1 @wrap_bool(0)
    %bad_false = cmp ne i1 %false, 0
    %bad_true = cmp ne i1 %true, 1
    %bad0 = binary or i1 %bad_unsigned, %bad_signed
    %bad1 = binary or i1 %bad_word, %bad_wide_unsigned
    %bad2 = binary or i1 %bad_wide_signed, %bad_false
    %bad3 = binary or i1 %bad0, %bad1
    %bad4 = binary or i1 %bad2, %bad_true
    %bad = binary or i1 %bad3, %bad4
    %exit = convert zext i32 i1 %bad
    return i32 %exit
}
