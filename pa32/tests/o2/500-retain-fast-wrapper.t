# Keep a short return path compact when the other path contains shared work.
function @shared_work(%x : i64) -> i64 [inline_hint=yes, binding=strong, unwind=no] {
  block ^entry:
    %w0 = binary mul i64 %x, 3
    %w1 = binary xor i64 %w0, 5
    %w2 = binary mul i64 %w1, 7
    %w3 = binary xor i64 %w2, 9
    %w4 = binary mul i64 %w3, 11
    %w5 = binary xor i64 %w4, 13
    %w6 = binary mul i64 %w5, 15
    %w7 = binary xor i64 %w6, 17
    %w8 = binary mul i64 %w7, 19
    %w9 = binary xor i64 %w8, 21
    %w10 = binary mul i64 %w9, 23
    %w11 = binary xor i64 %w10, 25
    %w12 = binary mul i64 %w11, 27
    %w13 = binary xor i64 %w12, 29
    %w14 = binary mul i64 %w13, 31
    %w15 = binary xor i64 %w14, 33
    %w16 = binary mul i64 %w15, 35
    %w17 = binary xor i64 %w16, 37
    %w18 = binary mul i64 %w17, 39
    %w19 = binary xor i64 %w18, 41
    %w20 = binary mul i64 %w19, 43
    %w21 = binary xor i64 %w20, 45
    %w22 = binary mul i64 %w21, 47
    %w23 = binary xor i64 %w22, 49
    %w24 = binary mul i64 %w23, 51
    %w25 = binary xor i64 %w24, 53
    %w26 = binary mul i64 %w25, 55
    %w27 = binary xor i64 %w26, 57
    %w28 = binary mul i64 %w27, 59
    %w29 = binary xor i64 %w28, 61
    %w30 = binary mul i64 %w29, 63
    %w31 = binary xor i64 %w30, 65
    %w32 = binary mul i64 %w31, 67
    %w33 = binary xor i64 %w32, 69
    %w34 = binary mul i64 %w33, 71
    %w35 = binary xor i64 %w34, 73
    %w36 = binary mul i64 %w35, 75
    %w37 = binary xor i64 %w36, 77
    %w38 = binary mul i64 %w37, 79
    %w39 = binary xor i64 %w38, 81
    %w40 = binary mul i64 %w39, 83
    %w41 = binary xor i64 %w40, 85
    %w42 = binary mul i64 %w41, 87
    %w43 = binary xor i64 %w42, 89
    %w44 = binary mul i64 %w43, 91
    %w45 = binary xor i64 %w44, 93
    %w46 = binary mul i64 %w45, 95
    %w47 = binary xor i64 %w46, 97
    return i64 %w47
}

function @wrapper(%fast : i32, %x : i64) -> i64 [no_inline=yes, binding=strong, unwind=no] {
  block ^entry:
    branch %fast, ^quick, ^slow
  block ^quick:
    return i64 %x
  block ^slow:
    %s0 = binary mul i64 %x, 5
    %s1 = binary xor i64 %s0, 7
    %s2 = binary mul i64 %s1, 9
    %s3 = binary xor i64 %s2, 11
    %s4 = binary mul i64 %s3, 13
    %s5 = binary xor i64 %s4, 15
    %s6 = binary mul i64 %s5, 17
    %s7 = binary xor i64 %s6, 19
    %s8 = binary mul i64 %s7, 21
    %s9 = binary xor i64 %s8, 23
    %s10 = binary mul i64 %s9, 25
    %s11 = binary xor i64 %s10, 27
    %s12 = binary mul i64 %s11, 29
    %s13 = binary xor i64 %s12, 31
    %s14 = binary mul i64 %s13, 33
    %s15 = binary xor i64 %s14, 35
    %s16 = binary mul i64 %s15, 37
    %s17 = binary xor i64 %s16, 39
    %s18 = binary mul i64 %s17, 41
    %s19 = binary xor i64 %s18, 43
    %s20 = binary mul i64 %s19, 45
    %s21 = binary xor i64 %s20, 47
    %s22 = binary mul i64 %s21, 49
    %s23 = binary xor i64 %s22, 51
    %s24 = binary mul i64 %s23, 53
    %s25 = binary xor i64 %s24, 55
    %s26 = binary mul i64 %s25, 57
    %s27 = binary xor i64 %s26, 59
    %s28 = binary mul i64 %s27, 61
    %s29 = binary xor i64 %s28, 63
    %s30 = binary mul i64 %s29, 65
    %s31 = binary xor i64 %s30, 67
    %s32 = binary mul i64 %s31, 69
    %s33 = binary xor i64 %s32, 71
    %s34 = binary mul i64 %s33, 73
    %s35 = binary xor i64 %s34, 75
    %result = call i64 @shared_work(%s35)
    return i64 %result
}

# The zero-iteration return does not make calls on the backedge cold.
function @hot_loop(%n : i64, %x : i64) -> i64 [no_inline=yes, binding=strong, unwind=no] {
  block ^entry:
    %empty = cmp eq i64 %n, 0
    branch %empty, ^done, ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %next]
    %sum = phi i64 [^entry: 0, ^loop: %added]
    %value = call i64 @shared_work(%x)
    %added = binary add i64 %sum, %value
    %next = binary add i64 %i, 1
    %more = cmp ne i64 %next, %n
    branch %more, ^loop, ^done
  block ^done:
    %result = phi i64 [^entry: 0, ^loop: %added]
    return i64 %result
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %fast = call i64 @wrapper(1, 7)
    %slow = call i64 @wrapper(0, 7)
    %bad_fast = cmp ne i64 %fast, 7
    %bad_slow = cmp ne i64 %slow, 3880516842696001031
    %zero = call i64 @hot_loop(0, 7)
    %five = call i64 @hot_loop(5, 7)
    %bad_zero = cmp ne i64 %zero, 0
    %bad_five = cmp ne i64 %five, 4822489477394739995
    %bad_wrapper = binary or i32 %bad_fast, %bad_slow
    %bad_loop = binary or i32 %bad_zero, %bad_five
    %bad = binary or i32 %bad_wrapper, %bad_loop
    return i32 %bad
}
