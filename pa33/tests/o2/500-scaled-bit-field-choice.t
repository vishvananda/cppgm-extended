function @scaled(%tag : i64, %base : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %bit = binary and i64 %tag, 2
    branch %bit, ^set, ^clear
  block ^set:
    %a = index i8 %base, 16
    %av = load i64 %a
    jump ^join
  block ^clear:
    %b = index i8 %base, 8
    %bv = load i64 %b
    jump ^join
  block ^join:
    %result = phi i64 [^set: %av, ^clear: %bv]
    return i64 %result
}

function @negative(%tag : i64, %base : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %bit = binary and i64 %tag, 1
    branch %bit, ^set, ^clear
  block ^set:
    %a = index i8 %base, -16
    %av = load i64 %a
    jump ^join
  block ^clear:
    %b = index i8 %base, -24
    %bv = load i64 %b
    jump ^join
  block ^join:
    %result = phi i64 [^set: %av, ^clear: %bv]
    return i64 %result
}

function @odd_bias(%tag : i64, %base : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %bit = binary and i64 %tag, 1
    branch %bit, ^set, ^clear
  block ^set:
    %a = index i8 %base, 1
    %av = load i64 %a
    jump ^join
  block ^clear:
    %b = index i8 %base, -7
    %bv = load i64 %b
    jump ^join
  block ^join:
    %result = phi i64 [^set: %av, ^clear: %bv]
    return i64 %result
}

function @main() -> i32 [role=entry] {
  slot $aligned : obj<64x8>
  slot $unaligned : obj<32x1>
  block ^entry:
    %base = addr $aligned
    %left = index i8 %base, 8
    %right = index i8 %base, 16
    store i64 101, %left
    store i64 303, %right
    %middle = index i8 %base, 32
    %a = call i64 @scaled(0, %base)
    %b = call i64 @scaled(2, %base)
    %c = call i64 @scaled(-1, %base)
    %d = call i64 @negative(0, %middle)
    %e = call i64 @negative(1, %middle)
    %odd = addr $unaligned
    %odd_left = index i8 %odd, 1
    %odd_right = index i8 %odd, 9
    store i64 45, %odd_left
    store i64 67, %odd_right
    %odd_middle = index i8 %odd, 8
    %f = call i64 @odd_bias(0, %odd_middle)
    %g = call i64 @odd_bias(1, %odd_middle)
    %bad_a = cmp ne i64 %a, 101
    %bad_b = cmp ne i64 %b, 303
    %bad_c = cmp ne i64 %c, 303
    %bad_d = cmp ne i64 %d, 101
    %bad_e = cmp ne i64 %e, 303
    %bad_f = cmp ne i64 %f, 45
    %bad_g = cmp ne i64 %g, 67
    %ab = binary or i64 %bad_a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %ef = binary or i64 %bad_e, %bad_f
    %abcd = binary or i64 %ab, %cd
    %efg = binary or i64 %ef, %bad_g
    %bad = binary or i64 %abcd, %efg
    %status = convert trunc i32 i64 %bad
    return i32 %status
}
