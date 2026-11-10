function @signed_quotient(%a : i32) -> i32 [unwind=no] {
  block ^entry:
    %result = binary div i32 %a, 8
    return i32 %result
}

function @signed_remainder(%a : i32) -> i32 [unwind=no] {
  block ^entry:
    %result = binary mod i32 %a, 8
    return i32 %result
}

function @negative_quotient(%a : i32) -> i32 [unwind=no] {
  block ^entry:
    %result = binary div i32 %a, -3
    return i32 %result
}

function @unsigned_quotient(%a : u32) -> u32 [unwind=no] {
  block ^entry:
    %result = binary udiv u32 %a, 3
    return u32 %result
}

function @unsigned_max_divisor(%a : u32) -> u32 [unwind=no] {
  block ^entry:
    %result = binary udiv u32 %a, 4294967295
    return u32 %result
}

function @unsigned_signed_op(%a : u32) -> u32 [unwind=no] {
  block ^entry:
    %result = binary div u32 %a, 8
    return u32 %result
}

function @signed_unsigned_op(%a : i32) -> i32 [unwind=no] {
  block ^entry:
    %result = binary udiv i32 %a, 3
    return i32 %result
}

function @negative_one(%a : i32) -> i32 [unwind=no] {
  block ^entry:
    %result = binary div i32 %a, -1
    return i32 %result
}

function @dynamic(%a : i32, %divisor : i32) -> i32 [unwind=no] {
  block ^entry:
    %result = binary div i32 %a, %divisor
    return i32 %result
}
function @from_pointer(%p : ptr) -> i32 [unwind=no] {
  block ^entry:
    %a = load i32 %p
    %result = binary div i32 %a, 8
    return i32 %result
}
function @main() -> i32 [role=entry, unwind=no] {
  slot $input : obj<16x8>
  block ^entry:
    %input = addr $input
    store i64 -2147483648, %input
    %value0 = call i32 @signed_quotient(-2147483648)
    %bad0 = cmp ne i32 %value0, -268435456
    %value1 = call i32 @signed_quotient(2147483647)
    %bad1 = cmp ne i32 %value1, 268435455
    %any1 = binary or i1 %bad0, %bad1
    %value2 = call i32 @signed_quotient(-31)
    %bad2 = cmp ne i32 %value2, -3
    %any2 = binary or i1 %any1, %bad2
    %value3 = call i32 @signed_remainder(-31)
    %bad3 = cmp ne i32 %value3, -7
    %any3 = binary or i1 %any2, %bad3
    %value4 = call i32 @negative_quotient(-2147483648)
    %bad4 = cmp ne i32 %value4, 715827882
    %any4 = binary or i1 %any3, %bad4
    %value5 = call u32 @unsigned_quotient(4294967295)
    %bad5 = cmp ne u32 %value5, 1431655765
    %any5 = binary or i1 %any4, %bad5
    %value6 = call u32 @unsigned_max_divisor(4294967295)
    %bad6 = cmp ne u32 %value6, 1
    %any6 = binary or i1 %any5, %bad6
    %value7 = call u32 @unsigned_max_divisor(4294967294)
    %bad7 = cmp ne u32 %value7, 0
    %any7 = binary or i1 %any6, %bad7
    %value8 = call u32 @unsigned_signed_op(2147483648)
    %bad8 = cmp ne u32 %value8, 4026531840
    %any8 = binary or i1 %any7, %bad8
    %value9 = call i32 @signed_unsigned_op(-1)
    %bad9 = cmp ne i32 %value9, 1431655765
    %any9 = binary or i1 %any8, %bad9
    %value10 = call i32 @negative_one(-2147483648)
    %bad10 = cmp ne i32 %value10, -2147483648
    %any10 = binary or i1 %any9, %bad10
    %value11 = call i32 @dynamic(-2147483648, -1)
    %bad11 = cmp ne i32 %value11, -2147483648
    %any11 = binary or i1 %any10, %bad11
    %value12 = call i32 @from_pointer(%input)
    %bad12 = cmp ne i32 %value12, -268435456
    %any12 = binary or i1 %any11, %bad12
    %exit = convert zext i32 i1 %any12
    return i32 %exit
}
