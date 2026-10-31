# A constant added to an index may join the address's displacement when only
# addresses read the sum, directly or through a byte scale. A sum also read as
# a value and a narrow sum that wraps must still see those exact values.
global @table = {
  i64 10
  i64 20
  i64 30
  i64 40
}
function @neighbors(%p : ptr, %i : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %before = binary sub i64 %i, 1
    %after = binary add i64 %i, 1
    %previous_at = index i64 %p, %before
    %next_at = index i64 %p, %after
    %previous = load i64 %previous_at
    %next = load i64 %next_at
    %result = binary sub i64 %next, %previous
    return i64 %result
}
function @byte_scaled(%p : ptr, %i : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %before = binary sub i64 %i, 1
    %bytes = binary mul i64 %before, 8
    %at = index i8 %p, %bytes
    %value = load i64 %at
    return i64 %value
}
function @shifted_bytes(%p : ptr, %i : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %after = binary add i64 %i, 2
    %bytes = binary shl i64 %after, 3
    %at = index i8 %p, %bytes
    %value = load i64 %at
    return i64 %value
}
function @negative_literal(%p : ptr, %i : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %before = binary add i64 %i, -2
    %at = index i32 %p, %before
    %value = load i32 %at
    %wide = convert sext i64 i32 %value
    return i64 %wide
}
function @literal_first(%p : ptr, %i : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %after = binary add i64 1, %i
    %at = index i16 %p, %after
    %value = load u16 %at
    %wide = convert zext i64 u16 %value
    return i64 %wide
}
function @other_block(%p : ptr, %i : i64, %which : i1) -> i64 [no_inline=yes] {
  block ^entry:
    %after = binary add i64 %i, 1
    %at = index i64 %p, %after
    branch %which, ^read, ^done

  block ^read:
    %value = load i64 %at
    return i64 %value

  block ^done:
    %address = copy i64 %at
    %distance = binary sub i64 %address, %p
    return i64 %distance
}
function @read_as_value(%p : ptr, %i : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %after = binary add i64 %i, 1
    %at = index i64 %p, %after
    %value = load i64 %at
    %result = binary add i64 %value, %after
    return i64 %result
}
function @scale_kept(%p : ptr, %i : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %after = binary add i64 %i, 1
    %bytes = binary mul i64 %after, 24
    %at = index i8 %p, %bytes
    %distance = binary sub i64 %at, %p
    return i64 %distance
}
function @narrow_wraps(%p : ptr, %i : i32) -> i64 [no_inline=yes] {
  block ^entry:
    %after = binary add i32 %i, 1
    %at = index i8 %p, %after
    %distance = binary sub i64 %at, %p
    return i64 %distance
}
function @wide_element(%p : ptr, %i : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %after = binary add i64 %i, 1
    %at = index obj<12x4> %p, %after
    %distance = binary sub i64 %at, %p
    return i64 %distance
}
function @symbol_base(%i : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %base = addr @table
    %after = binary add i64 %i, 1
    %at = index i64 %base, %after
    %value = load i64 %at
    return i64 %value
}
function @frame_base(%i : i64) -> i64 [no_inline=yes] {
  slot $cells : obj<32x8>
  block ^entry:
    %zero = addr $cells
    store i64 5, %zero
    %one = index i64 %zero, 1
    store i64 6, %one
    %two = index i64 %zero, 2
    store i64 7, %two
    %three = index i64 %zero, 3
    store i64 8, %three
    %after = binary add i64 %i, 2
    %at = index i64 %zero, %after
    %value = load i64 %at
    return i64 %value
}
function @main() -> i64 [role=entry] {
  block ^entry:
    %table = addr @table
    %third = index i64 %table, 2
    %a = call i64 @neighbors(%table, 1)
    %bad_a = cmp ne i64 %a, 20
    %b = call i64 @byte_scaled(%table, 3)
    %bad_b = cmp ne i64 %b, 30
    %c = call i64 @shifted_bytes(%table, 1)
    %bad_c = cmp ne i64 %c, 40
    %d = call i64 @negative_literal(%table, 6)
    %bad_d = cmp ne i64 %d, 30
    %e = call i64 @literal_first(%table, 7)
    %bad_e = cmp ne i64 %e, 30
    %f = call i64 @other_block(%table, 1, 1)
    %bad_f = cmp ne i64 %f, 30
    %g = call i64 @other_block(%table, 1, 0)
    %bad_g = cmp ne i64 %g, 16
    %h = call i64 @read_as_value(%table, 0)
    %bad_h = cmp ne i64 %h, 21
    %i = call i64 @scale_kept(%table, 2)
    %bad_i = cmp ne i64 %i, 72
    %j = call i64 @narrow_wraps(%table, 2147483647)
    %bad_j = cmp ne i64 %j, -2147483648
    %k = call i64 @wide_element(%table, 2)
    %bad_k = cmp ne i64 %k, 36
    %l = call i64 @symbol_base(1)
    %bad_l = cmp ne i64 %l, 30
    %m = call i64 @frame_base(1)
    %bad_m = cmp ne i64 %m, 8
    %n = call i64 @neighbors(%third, -1)
    %bad_n = cmp ne i64 %n, 20
    %s0 = binary or i64 %bad_a, %bad_b
    %s1 = binary or i64 %s0, %bad_c
    %s2 = binary or i64 %s1, %bad_d
    %s3 = binary or i64 %s2, %bad_e
    %s4 = binary or i64 %s3, %bad_f
    %s5 = binary or i64 %s4, %bad_g
    %s6 = binary or i64 %s5, %bad_h
    %s7 = binary or i64 %s6, %bad_i
    %s8 = binary or i64 %s7, %bad_j
    %s9 = binary or i64 %s8, %bad_k
    %s10 = binary or i64 %s9, %bad_l
    %s11 = binary or i64 %s10, %bad_m
    %failed = binary or i64 %s11, %bad_n
    %status = convert trunc i32 i64 %failed
    return i64 %status
}
