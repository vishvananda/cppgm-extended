# A select of one bit or zero on a test of one bit of a value is that bit
# moved to its place: a shift and a mask, no test. Terms of an `or` that keep
# bits of one value keep them under one mask.
global @seen : i64 = 0

# (x & 8) ? 8 : 0: the bit where it is.
function @in_place(%x : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and u32 %x, 8
    %r = select u32 %a, 8, 0
    return u32 %r
}

# (x & 8) ? 32 : 0: two places up.
function @up(%x : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and u32 %x, 8
    %c = cmp ne u32 %a, 0
    %r = select u32 %c, 32, 0
    return u32 %r
}

# (x & 64) ? 2 : 0 in a narrower type: five places down, then narrowed.
function @down(%x : i64) -> u8 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and i64 %x, 64
    %r = select u8 %a, 2, 0
    return u8 %r
}

# (x & 128) ? 1 << 40 : 0: widened before the shift.
function @wider(%x : u8) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and u8 %x, 128
    %r = select i64 %a, 1099511627776, 0
    return i64 %r
}

# (x & 4) ? 0 : 4 and (x & 4) == 0 ? 0 : 16: the bit, the first inverted.
function @clear(%x : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and u32 %x, 4
    %r = select u32 %a, 0, 4
    %c = cmp eq u32 %a, 0
    %s = select u32 %c, 0, 16
    %t = binary or u32 %r, %s
    return u32 %t
}

# (x & 1) | (x & 6) | (x & 64): one mask.
function @union(%x : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and u32 %x, 1
    %b = binary and u32 %x, 6
    %c = binary and u32 %x, 64
    %d = binary or u32 %a, %b
    %e = binary or u32 %d, %c
    return u32 %e
}

# The same under zero-extensions, on either side of the mask.
function @extended(%x : u8) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %w = convert zext u32 u8 %x
    %a = binary and u8 %x, 1
    %b = convert zext u32 u8 %a
    %c = binary and u32 %w, 4
    %d = binary or u32 %b, %c
    return u32 %d
}

# ((x & 1) ? 4 : 0) | ((x & 2) ? 8 : 0) | (y & 3): one shift and one mask
# for x's bits, beside the term of another value.
function @moved(%x : u32, %y : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and u32 %x, 1
    %b = select u32 %a, 4, 0
    %c = binary and u32 %x, 2
    %d = select u32 %c, 8, 0
    %e = binary and u32 %y, 3
    %f = binary or u32 %b, %e
    %g = binary or u32 %f, %d
    return u32 %g
}

# The selects stay: a test of two bits, and a value of two bits.
function @two_tested(%x : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and u32 %x, 12
    %r = select u32 %a, 4, 0
    return u32 %r
}

function @two_set(%x : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and u32 %x, 4
    %r = select u32 %a, 6, 0
    return u32 %r
}

# The masks stay: terms of two values, and terms read again after the `or`.
function @two_values(%x : u32, %y : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and u32 %x, 1
    %b = binary and u32 %y, 6
    %c = binary or u32 %a, %b
    return u32 %c
}

function @read_again(%x : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and u32 %x, 1
    %b = binary and u32 %x, 6
    %c = binary or u32 %a, %b
    %d = binary mul u32 %a, %b
    %e = binary add u32 %c, %d
    return u32 %e
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call u32 @in_place(12)
    %b = call u32 @up(8)
    %c = call u8 @down(64)
    %d = call i64 @wider(255)
    %e = call u32 @clear(3)
    %f = call u32 @union(255)
    %g = call u32 @extended(7)
    %h = call u32 @moved(3, 2)
    %i = call u32 @two_tested(8)
    %j = call u32 @two_set(4)
    %k = call u32 @two_values(3, 6)
    %l = call u32 @read_again(7)
    %m = binary add u32 %a, %b
    %n = convert zext u32 u8 %c
    %o = binary add u32 %m, %n
    %p = binary ushr i64 %d, 36
    %q = convert trunc u32 i64 %p
    %r = binary add u32 %o, %q
    %s = binary add u32 %e, %f
    %u = binary add u32 %s, %g
    %v = binary add u32 %h, %i
    %w = binary add u32 %j, %k
    %x = binary add u32 %v, %w
    %y = binary add u32 %r, %u
    %z = binary add u32 %x, %y
    %aa = binary add u32 %z, %l
    %ab = copy i32 %aa
    return i32 %ab
}
