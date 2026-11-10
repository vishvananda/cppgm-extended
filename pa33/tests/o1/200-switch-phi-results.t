# Dense constant cases keep defined defaults, side effects and power facts.
global @effects : i64 = 0

function @lookup(%n : i64, %seed : i64) -> i64 [no_inline=yes] {
  block ^entry:
    switch %n, ^otherwise, 0:^case0, 1:^case1, 2:^case2, 3:^case3, 4:^case4, 5:^case5, 6:^case6, 7:^case7, 8:^case8
  block ^case0:
    jump ^merge
  block ^case1:
    jump ^merge
  block ^case2:
    jump ^merge
  block ^case3:
    %old = load volatile i64 @effects
    %next = binary add i64 %old, %seed
    store volatile i64 %next, @effects
    %choice = binary xor i64 %seed, 45
    jump ^merge
  block ^case4:
    jump ^merge
  block ^case5:
    jump ^merge
  block ^case6:
    jump ^merge
  block ^case7:
    jump ^merge
  block ^case8:
    jump ^merge
  block ^otherwise:
    %fallback = binary add i64 %seed, 99
    jump ^merge
  block ^merge:
    %result = phi i64 [^case0: 3, ^case1: 17, ^case2: 8, ^case3: %choice, ^case4: 9, ^case5: 5, ^case6: 4, ^case7: 1, ^case8: 11, ^otherwise: %fallback]
    return i64 %result
}

function @holes(%n : i64, %seed : i64) -> i64 [no_inline=yes] {
  block ^entry:
    switch %n, ^otherwise, -5:^case0, -3:^case1, -1:^case2, 1:^case3, 3:^case4, 5:^case5
  block ^case0:
    jump ^merge
  block ^case1:
    jump ^merge
  block ^case2:
    jump ^merge
  block ^case3:
    jump ^merge
  block ^case4:
    jump ^merge
  block ^case5:
    jump ^merge
  block ^otherwise:
    jump ^merge
  block ^merge:
    %result = phi i64 [^case0: -17, ^case1: 33, ^case2: 46, ^case3: 97, ^case4: 109, ^case5: 120, ^otherwise: %seed]
    return i64 %result
}

function @powers(%n : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    switch %n, ^otherwise, 0:^case0, 1:^case1, 2:^case2, 3:^case3, 4:^case4, 5:^case5, 6:^case6, 7:^case7, 8:^case8
  block ^case0:
    jump ^merge
  block ^case1:
    jump ^merge
  block ^case2:
    jump ^merge
  block ^case3:
    jump ^merge
  block ^case4:
    jump ^merge
  block ^case5:
    jump ^merge
  block ^case6:
    jump ^merge
  block ^case7:
    jump ^merge
  block ^case8:
    jump ^merge
  block ^otherwise:
    jump ^merge
  block ^merge:
    %result = phi i64 [^case0: 1, ^case1: 2, ^case2: 4, ^case3: 8, ^case4: 16, ^case5: 32, ^case6: 64, ^case7: 128, ^case8: 256, ^otherwise: 512]
    return i64 %result
}

function @round_through_table(%x : i64, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %alignment = call i64 @powers(%n)
    %nonzero = cmp ne i64 %x, 0
    %bias = convert zext i64 i1 %nonzero
    %start = binary sub i64 %x, %bias
    %quotient = binary udiv i64 %start, %alignment
    %ceiling = binary add i64 %quotient, %bias
    %result = binary mul i64 %ceiling, %alignment
    return i64 %result
}

function @duplicates(%n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    switch %n, ^otherwise, 0:^a, 1:^b, 2:^c, 3:^d, 4:^e, 5:^f, 3:^duplicate
  block ^a:
    jump ^merge
  block ^b:
    jump ^merge
  block ^c:
    jump ^merge
  block ^d:
    jump ^merge
  block ^e:
    jump ^merge
  block ^f:
    jump ^merge
  block ^duplicate:
    jump ^merge
  block ^otherwise:
    jump ^merge
  block ^merge:
    %result = phi i64 [^a: 11, ^b: 17, ^c: 23, ^d: 33, ^e: 49, ^f: 61, ^duplicate: 77, ^otherwise: 99]
    return i64 %result
}

function @shift_word(%n : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %result = binary shl i32 1, %n
    return i32 %result
}
function @signed_word_division(%x : i64, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    %word = call i32 @shift_word(%n)
    %divisor = convert sext i64 i32 %word
    %result = binary udiv i64 %x, %divisor
    return i64 %result
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %signed = call i64 @signed_word_division(-1, 31)
    %bad_signed = cmp ne i64 %signed, 1
    %duplicate = call i64 @duplicates(3)
    %bad_duplicate = cmp ne i64 %duplicate, 33
    %a = call i64 @lookup(1, 7)
    %b = call i64 @lookup(3, 7)
    %c = call i64 @lookup(99, 13)
    %d = call i64 @holes(-3, 901)
    %e = call i64 @holes(-4, 901)
    %f = call i64 @holes(-5, 901)
    %g = call i64 @round_through_table(123, 3)
    %h = call i64 @round_through_table(-1, 8)
    %i = call i64 @powers(99)
    %ticks = load volatile i64 @effects
    %bad_a = cmp ne i64 %a, 17
    %bad_b = cmp ne i64 %b, 42
    %bad_c = cmp ne i64 %c, 112
    %bad_d = cmp ne i64 %d, 33
    %bad_e = cmp ne i64 %e, 901
    %bad_f = cmp ne i64 %f, -17
    %bad_g = cmp ne i64 %g, 128
    %bad_h = cmp ne i64 %h, 0
    %bad_i = cmp ne i64 %i, 512
    %bad_ticks = cmp ne i64 %ticks, 7
    %ab = binary or i64 %bad_a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %ef = binary or i64 %bad_e, %bad_f
    %gh = binary or i64 %bad_g, %bad_h
    %it = binary or i64 %bad_i, %bad_ticks
    %abcd = binary or i64 %ab, %cd
    %efgh = binary or i64 %ef, %gh
    %all = binary or i64 %abcd, %efgh
    %bad = binary or i64 %all, %it
    %checked = binary or i64 %bad, %bad_duplicate
    %all_checked = binary or i64 %checked, %bad_signed
    return i64 %all_checked
}
