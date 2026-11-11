# The live values put pressure on the promoted byte and reused mask.
# Reloading one tested operand must preserve the other, in either branch.
global @values : obj<48x4> = { zero 48 }
global @byte : obj<1x1> = { zero 1 }
function @check(%input : ptr, %bit : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %mask = binary shl i32 1, %bit
    %byte = load u8 %input
    %wide = convert zext i32 u8 %byte
    %p0 = index i32 @values, 0
    %v0 = load typed i32 %p0
    %p1 = index i32 @values, 1
    %v1 = load typed i32 %p1
    %p2 = index i32 @values, 2
    %v2 = load typed i32 %p2
    %p3 = index i32 @values, 3
    %v3 = load typed i32 %p3
    %p4 = index i32 @values, 4
    %v4 = load typed i32 %p4
    %p5 = index i32 @values, 5
    %v5 = load typed i32 %p5
    %p6 = index i32 @values, 6
    %v6 = load typed i32 %p6
    %p7 = index i32 @values, 7
    %v7 = load typed i32 %p7
    %p8 = index i32 @values, 8
    %v8 = load typed i32 %p8
    %p9 = index i32 @values, 9
    %v9 = load typed i32 %p9
    %p10 = index i32 @values, 10
    %v10 = load typed i32 %p10
    %p11 = index i32 @values, 11
    %v11 = load typed i32 %p11
    %bits = binary and i32 %mask, %wide
    branch %bits, ^yes, ^no
  block ^yes:
    return i32 99
  block ^no:
    store typed i32 %mask, @values
    store typed i32 %v0, @values
    store typed i32 %v1, @values
    store typed i32 %v2, @values
    store typed i32 %v3, @values
    store typed i32 %v4, @values
    store typed i32 %v5, @values
    store typed i32 %v6, @values
    store typed i32 %v7, @values
    store typed i32 %v8, @values
    store typed i32 %v9, @values
    store typed i32 %v10, @values
    store typed i32 %v11, @values
    store typed i32 %v0, @values
    store typed i32 %v1, @values
    store typed i32 %v2, @values
    store typed i32 %v3, @values
    store typed i32 %v4, @values
    store typed i32 %v5, @values
    store typed i32 %v6, @values
    store typed i32 %v7, @values
    store typed i32 %v8, @values
    store typed i32 %v9, @values
    store typed i32 %v10, @values
    store typed i32 %v11, @values
    store typed i32 %v0, @values
    store typed i32 %v1, @values
    store typed i32 %v2, @values
    store typed i32 %v3, @values
    store typed i32 %v4, @values
    store typed i32 %v5, @values
    store typed i32 %v6, @values
    store typed i32 %v7, @values
    store typed i32 %v8, @values
    store typed i32 %v9, @values
    store typed i32 %v10, @values
    store typed i32 %v11, @values
    store typed i32 %v0, @values
    store typed i32 %v1, @values
    store typed i32 %v2, @values
    store typed i32 %v3, @values
    store typed i32 %v4, @values
    store typed i32 %v5, @values
    store typed i32 %v6, @values
    store typed i32 %v7, @values
    store typed i32 %v8, @values
    store typed i32 %v9, @values
    store typed i32 %v10, @values
    store typed i32 %v11, @values
    return i32 0
}
function @main() -> i32 [role=entry] {
  block ^entry:
    %zero = call i32 @check(@byte, 0)
    store u8 128, @byte
    %clear = call i32 @check(@byte, 0)
    %set = call i32 @check(@byte, 7)
    %upper = call i32 @check(@byte, 8)
    %bad_zero = cmp ne i32 %zero, 0
    %bad_clear = cmp ne i32 %clear, 0
    %bad_set = cmp ne i32 %set, 99
    %bad_upper = cmp ne i32 %upper, 0
    %bad_low = binary or i32 %bad_zero, %bad_clear
    %bad_high = binary or i32 %bad_set, %bad_upper
    %bad = binary or i32 %bad_low, %bad_high
    return i32 %bad
}
