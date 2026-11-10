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
function @branch_byte(%a : i64, %b : i64) -> i32 [unwind=no] {
  block ^entry:
    %result = call i1 @signed_byte(%a, %b)
    branch %result, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @branch_wide(%a : i64, %b : i64) -> i32 [unwind=no] {
  block ^entry:
    %result = call i1 @signed_byte(%a, %b)
    %wide = convert zext i64 i1 %result
    branch %wide, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
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
    %value11 = call i32 @branch_byte(-2, 1)
    %bad11 = cmp ne i32 %value11, 1
    %any11 = binary or i1 %any10, %bad11
    %value12 = call i32 @branch_byte(2, 1)
    %bad12 = cmp ne i32 %value12, 0
    %any12 = binary or i1 %any11, %bad12
    %value13 = call i32 @branch_wide(-2, 1)
    %bad13 = cmp ne i32 %value13, 1
    %any13 = binary or i1 %any12, %bad13
    %value14 = call i32 @branch_wide(2, 1)
    %bad14 = cmp ne i32 %value14, 0
    %any14 = binary or i1 %any13, %bad14
    %exit = convert zext i32 i1 %any14
    return i32 %exit
}
