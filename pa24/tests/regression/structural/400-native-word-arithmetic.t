function @unsigned_add(%a : u32, %b : u32) -> i64 [unwind=no] {
  block ^entry:
    %value = binary add u32 %a, %b
    %wide = convert zext i64 u32 %value
    return i64 %wide
}

function @unsigned_sub(%a : u32, %b : u32) -> i64 [unwind=no] {
  block ^entry:
    %value = binary sub u32 %a, %b
    %wide = convert zext i64 u32 %value
    return i64 %wide
}

function @unsigned_mul(%a : u32, %b : u32) -> i64 [unwind=no] {
  block ^entry:
    %value = binary mul u32 %a, %b
    %wide = convert zext i64 u32 %value
    return i64 %wide
}

function @signed_add(%a : i32, %b : i32) -> i64 [unwind=no] {
  block ^entry:
    %value = binary add i32 %a, %b
    %wide = convert sext i64 i32 %value
    return i64 %wide
}

function @signed_sub(%a : i32, %b : i32) -> i64 [unwind=no] {
  block ^entry:
    %value = binary sub i32 %a, %b
    %wide = convert sext i64 i32 %value
    return i64 %wide
}

function @signed_mul(%a : i32, %b : i32) -> i64 [unwind=no] {
  block ^entry:
    %value = binary mul i32 %a, %b
    %wide = convert sext i64 i32 %value
    return i64 %wide
}
function @main() -> i32 [role=entry, unwind=no] {
  block ^entry:
    %value0 = call i64 @unsigned_add(4294967295, 1)
    %bad0 = cmp ne i64 %value0, 0
    %value1 = call i64 @unsigned_add(2147483647, 1)
    %bad1 = cmp ne i64 %value1, 2147483648
    %any1 = binary or i1 %bad0, %bad1
    %value2 = call i64 @unsigned_sub(0, 1)
    %bad2 = cmp ne i64 %value2, 4294967295
    %any2 = binary or i1 %any1, %bad2
    %value3 = call i64 @unsigned_mul(4294967294, 3)
    %bad3 = cmp ne i64 %value3, 4294967290
    %any3 = binary or i1 %any2, %bad3
    %value4 = call i64 @signed_add(2147483647, 1)
    %bad4 = cmp ne i64 %value4, -2147483648
    %any4 = binary or i1 %any3, %bad4
    %value5 = call i64 @signed_add(-2147483648, -1)
    %bad5 = cmp ne i64 %value5, 2147483647
    %any5 = binary or i1 %any4, %bad5
    %value6 = call i64 @signed_sub(-2147483648, 1)
    %bad6 = cmp ne i64 %value6, 2147483647
    %any6 = binary or i1 %any5, %bad6
    %value7 = call i64 @signed_mul(1073741824, 2)
    %bad7 = cmp ne i64 %value7, -2147483648
    %any7 = binary or i1 %any6, %bad7
    %value8 = call i64 @signed_mul(-5, 3)
    %bad8 = cmp ne i64 %value8, -15
    %any8 = binary or i1 %any7, %bad8
    %exit = convert zext i32 i1 %any8
    return i32 %exit
}
