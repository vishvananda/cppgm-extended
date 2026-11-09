# Zero-extended masked truth values still identify a bit recurrence.
function @fold_widened(%x : u8) -> u8 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and u8 %x, 1
    %cond0 = convert zext i64 u8 %bit0
    %s0 = binary ushr u8 %x, 1
    %m0 = binary xor u8 %s0, 184
    %r0 = select u8 %cond0, %m0, %s0
    %bit1 = binary and u8 %r0, 1
    %cond1 = convert zext i64 u8 %bit1
    %s1 = binary ushr u8 %r0, 1
    %m1 = binary xor u8 %s1, 184
    %r1 = select u8 %cond1, %m1, %s1
    %bit2 = binary and u8 %r1, 1
    %cond2 = convert zext i64 u8 %bit2
    %s2 = binary ushr u8 %r1, 1
    %m2 = binary xor u8 %s2, 184
    %r2 = select u8 %cond2, %m2, %s2
    %bit3 = binary and u8 %r2, 1
    %cond3 = convert zext i64 u8 %bit3
    %s3 = binary ushr u8 %r2, 1
    %m3 = binary xor u8 %s3, 184
    %r3 = select u8 %cond3, %m3, %s3
    return u8 %r3
}
function @keep_second_bit(%x : u8) -> u8 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and u8 %x, 2
    %cond0 = convert zext i64 u8 %bit0
    %s0 = binary ushr u8 %x, 1
    %m0 = binary xor u8 %s0, 184
    %r0 = select u8 %cond0, %m0, %s0
    %bit1 = binary and u8 %r0, 2
    %cond1 = convert zext i64 u8 %bit1
    %s1 = binary ushr u8 %r0, 1
    %m1 = binary xor u8 %s1, 184
    %r1 = select u8 %cond1, %m1, %s1
    %bit2 = binary and u8 %r1, 2
    %cond2 = convert zext i64 u8 %bit2
    %s2 = binary ushr u8 %r1, 1
    %m2 = binary xor u8 %s2, 184
    %r2 = select u8 %cond2, %m2, %s2
    %bit3 = binary and u8 %r2, 2
    %cond3 = convert zext i64 u8 %bit3
    %s3 = binary ushr u8 %r2, 1
    %m3 = binary xor u8 %s3, 184
    %r3 = select u8 %cond3, %m3, %s3
    return u8 %r3
}
function @main() -> i64 [role=entry] {
  block ^entry:
    jump ^outer
  block ^outer:
    %i = phi i64 [^entry: 0, ^next: %inext]
    %seed = convert trunc u8 i64 %i
    %actual_a = call u8 @fold_widened(%seed)
    %actual_b = call u8 @keep_second_bit(%seed)
    jump ^inner
  block ^inner:
    %count = phi i64 [^outer: 0, ^step: %countnext]
    %a = phi u8 [^outer: %seed, ^step: %anext]
    %b = phi u8 [^outer: %seed, ^step: %bnext]
    %abit = binary and u8 %a, 1
    %bbit = binary and u8 %b, 2
    %acond = cmp ne u8 %abit, 0
    %bcond = cmp ne u8 %bbit, 0
    %ashift = binary ushr u8 %a, 1
    %bshift = binary ushr u8 %b, 1
    %axor = binary xor u8 %ashift, 184
    %bxor = binary xor u8 %bshift, 184
    %anext = select u8 %acond, %axor, %ashift
    %bnext = select u8 %bcond, %bxor, %bshift
    %countnext = binary add i64 %count, 1
    %more = cmp ne i64 %countnext, 4
    branch %more, ^step, ^verify
  block ^step:
    jump ^inner
  block ^verify:
    %bad_a = cmp ne u8 %actual_a, %anext
    %bad_b = cmp ne u8 %actual_b, %bnext
    %bad = binary or i64 %bad_a, %bad_b
    branch %bad, ^fail, ^next
  block ^next:
    %inext = binary add i64 %i, 1
    %again = cmp ne i64 %inext, 256
    branch %again, ^outer, ^done
  block ^fail:
    return i64 1
  block ^done:
    return i64 0
}
