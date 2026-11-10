function @dirty() -> i64 [unwind=no] {
  block ^entry:
    return i64 -1
}
function @signed_byte(%a : i64, %b : i64) -> i1 [unwind=no] {
  block ^entry:
    %dirty = call i64 @dirty()
    %result = cmp lt i64 %a, %b
    return i1 %result
}
function @unsigned_byte(%a : i64, %b : i64) -> i1 [unwind=no] {
  block ^entry:
    %result = cmp ult i64 %a, %b
    return i1 %result
}
function @pointer_byte(%p : ptr) -> i1 [unwind=no] {
  block ^entry:
    %result = cmp ne ptr %p, 0
    return i1 %result
}
function @forward_byte(%a : i64, %b : i64) -> i1 [unwind=no] {
  block ^entry:
    %result = call i1 @signed_byte(%a, %b)
    return i1 %result
}
function @signed_wide(%a : i64, %b : i64) -> i64 [unwind=no] {
  block ^entry:
    %dirty = call i64 @dirty()
    %result = cmp lt i64 %a, %b
    %wide = convert zext i64 i1 %result
    return i64 %wide
}
function @forward_wide(%a : i64, %b : i64) -> i64 [unwind=no] {
  block ^entry:
    %result = call i1 @forward_byte(%a, %b)
    %wide = convert zext i64 i1 %result
    return i64 %wide
}
function @main() -> i32 [role=entry, unwind=no] {
  slot $input : obj<16x8>
  block ^entry:
    %input = addr $input
    store i64 -2147483648, %input
    %value0 = call i1 @signed_byte(-2, 1)
    %bad0 = cmp ne i1 %value0, 1
    %value1 = call i1 @signed_byte(2, 1)
    %bad1 = cmp ne i1 %value1, 0
    %any1 = binary or i1 %bad0, %bad1
    %value2 = call i1 @unsigned_byte(18446744073709551615, 0)
    %bad2 = cmp ne i1 %value2, 0
    %any2 = binary or i1 %any1, %bad2
    %value3 = call i1 @unsigned_byte(0, 18446744073709551615)
    %bad3 = cmp ne i1 %value3, 1
    %any3 = binary or i1 %any2, %bad3
    %value4 = call i1 @pointer_byte(0)
    %bad4 = cmp ne i1 %value4, 0
    %any4 = binary or i1 %any3, %bad4
    %value5 = call i1 @pointer_byte(%input)
    %bad5 = cmp ne i1 %value5, 1
    %any5 = binary or i1 %any4, %bad5
    %value6 = call i1 @forward_byte(2, 1)
    %bad6 = cmp ne i1 %value6, 0
    %any6 = binary or i1 %any5, %bad6
    %value7 = call i64 @signed_wide(-2, 1)
    %bad7 = cmp ne i64 %value7, 1
    %any7 = binary or i1 %any6, %bad7
    %value8 = call i64 @signed_wide(2, 1)
    %bad8 = cmp ne i64 %value8, 0
    %any8 = binary or i1 %any7, %bad8
    %value9 = call i64 @forward_wide(-2, 1)
    %bad9 = cmp ne i64 %value9, 1
    %any9 = binary or i1 %any8, %bad9
    %value10 = call i64 @forward_wide(2, 1)
    %bad10 = cmp ne i64 %value10, 0
    %any10 = binary or i1 %any9, %bad10
    %exit = convert zext i32 i1 %any10
    return i32 %exit
}
