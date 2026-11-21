# A difference of two values that steps and scalings by literals in one
# type lead to one value, alike scaled, is a literal: `@step` is 8,
# `@scaled_step` 64 (both scaled by 8 after a zero extension), and
# `@wrapping` wraps as i32 does, to -2147483648. Two element addresses of
# one root whose 64-bit indexes differ so are a literal distance apart:
# `@byte_elements` 24 bytes, `@typed_elements` 40 bytes (5 doubles), so that
# a copy's length (`@copy_length`, 3 doubles) is known. Scales that differ
# (`@different_scales`) and a step in a narrower type before the extension
# (`@narrow_step`, which wraps at the u32 limit) keep the subtraction.
function @step(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %a = binary add i64 %x, 8
    %d = binary sub i64 %a, %x
    return i64 %d
}

function @scaled_step(%x : u32) -> i64 [no_inline=yes] {
  block ^entry:
    %k = convert zext i64 u32 %x
    %next = binary add i64 %k, 8
    %high = binary mul i64 %next, 8
    %low = binary shl i64 %k, 3
    %d = binary sub i64 %high, %low
    return i64 %d
}

function @wrapping(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %a = binary add i32 %x, 2147483647
    %b = binary sub i32 %x, 1
    %d = binary sub i32 %a, %b
    return i32 %d
}

function @byte_elements(%p : ptr, %k : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %next = binary add i64 %k, 3
    %high = binary mul i64 %next, 8
    %low = binary mul i64 %k, 8
    %e = index i8 %p, %high
    %b = index i8 %p, %low
    %d = binary sub ptr %e, %b
    return i64 %d
}

function @typed_elements(%p : ptr, %k : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %far = binary add i64 %k, 5
    %e = index f64 %p, %far
    %b = index f64 %p, %k
    %d = binary sub ptr %e, %b
    return i64 %d
}

function @copy_length(%p : ptr, %k : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %base = index i8 %p, 16
    %far = binary add i64 %k, 3
    %e = index f64 %base, %far
    %b = index f64 %base, %k
    %bytes = binary sub ptr %e, %b
    %n = binary shr exact i64 %bytes, 3
    return i64 %n
}

function @different_scales(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %twice = binary mul i64 %x, 2
    %a = binary add i64 %twice, 1
    %d = binary sub i64 %a, %x
    return i64 %d
}

function @narrow_step(%x : u32) -> i64 [no_inline=yes] {
  block ^entry:
    %a = binary add u32 %x, 1
    %wide_a = convert zext i64 u32 %a
    %wide_x = convert zext i64 u32 %x
    %d = binary sub i64 %wide_a, %wide_x
    return i64 %d
}

function @main() -> i64 [role=entry] {
  slot $array : obj<128x8>
  block ^entry:
    %p = addr $array
    %s = call i64 @step(-5)
    %c = call i64 @scaled_step(4294967295)
    %w = call i32 @wrapping(7)
    %y = call i64 @byte_elements(%p, 2)
    %t = call i64 @typed_elements(%p, 1)
    %n = call i64 @copy_length(%p, 4)
    %g = call i64 @different_scales(10)
    %m = call i64 @narrow_step(4294967295)
    %m1 = call i64 @narrow_step(6)
    %sb = cmp ne i64 %s, 8
    %cb = cmp ne i64 %c, 64
    %wb = cmp ne i32 %w, -2147483648
    %yb = cmp ne i64 %y, 24
    %tb = cmp ne i64 %t, 40
    %nb = cmp ne i64 %n, 3
    %gb = cmp ne i64 %g, 11
    %mb = cmp ne i64 %m, -4294967295
    %m1b = cmp ne i64 %m1, 1
    %r1 = binary or i64 %sb, %cb
    %r2 = binary or i64 %r1, %wb
    %r3 = binary or i64 %r2, %yb
    %r4 = binary or i64 %r3, %tb
    %r5 = binary or i64 %r4, %nb
    %r6 = binary or i64 %r5, %gb
    %r7 = binary or i64 %r6, %mb
    %bad = binary or i64 %r7, %m1b
    return i64 %bad
}
