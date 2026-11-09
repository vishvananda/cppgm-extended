# A one-bit mask already yields a truth value. Wider masks, arbitrary XOR
# values and sign-extending conversions still require normalization.
function @masked(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %bit = binary and i64 %x, 1
    %truth = cmp ne i64 %bit, 0
    return i64 %truth
}

function @high_bit(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %shifted = binary shr i64 %x, 31
    %bit = binary and i64 %shifted, 1
    %truth = cmp eq i64 %bit, 1
    return i64 %truth
}

function @wide_mask(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %bits = binary and i64 %x, 2
    %truth = cmp ne i64 %bits, 0
    return i64 %truth
}

function @arbitrary_xor(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %bits = binary xor i64 %x, 1
    %truth = cmp ne i64 %bits, 0
    return i64 %truth
}

function @signed_byte(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %byte = convert trunc i8 i64 %x
    %wide = convert sext i64 i8 %byte
    %truth = cmp ne i64 %wide, 0
    return i64 %truth
}

function @volatile_bit(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %value = load volatile i64 %p
    %bit = binary and i64 %value, 1
    %truth = cmp ne i64 %bit, 0
    return i64 %truth
}

function @main() -> i64 [role=entry] {
  slot $cell : i64
  block ^entry:
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^next: %inext]
    %high = binary shl i64 %i, 31
    %both = binary or i64 %high, %i
    %value = binary xor i64 %both, -9223372036854775808
    %expected = binary and i64 %i, 1
    %a = call i64 @masked(%value)
    %b = call i64 @high_bit(%value)
    %c = call i64 @wide_mask(%value)
    %d = call i64 @arbitrary_xor(%i)
    %e = call i64 @signed_byte(%i)
    store i64 %value, $cell
    %p = addr $cell
    %f = call i64 @volatile_bit(%p)
    %masked2 = binary and i64 %i, 2
    %expected2 = cmp ne i64 %masked2, 0
    %changed = binary xor i64 %i, 1
    %expected_xor = cmp ne i64 %changed, 0
    %low = binary and i64 %i, 255
    %expected_byte = cmp ne i64 %low, 0
    %bad_a = cmp ne i64 %a, %expected
    %bad_b = cmp ne i64 %b, %expected
    %bad_c = cmp ne i64 %c, %expected2
    %bad_d = cmp ne i64 %d, %expected_xor
    %bad_e = cmp ne i64 %e, %expected_byte
    %bad_f = cmp ne i64 %f, %expected
    %ab = binary or i64 %bad_a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %ef = binary or i64 %bad_e, %bad_f
    %abcd = binary or i64 %ab, %cd
    %bad = binary or i64 %abcd, %ef
    branch %bad, ^fail, ^next
  block ^next:
    %inext = binary add i64 %i, 1
    %more = cmp ne i64 %inext, 1024
    branch %more, ^loop, ^done
  block ^fail:
    return i64 1
  block ^done:
    return i64 0
}
