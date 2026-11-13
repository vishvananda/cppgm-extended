# A 32-bit or 16-bit immediate whose bits are a sign-extended byte
# (4294967295 in a u32 compare, 65520 in a u16 mask) takes the instruction's
# byte form. The operations read the same value either way: `@last` tells the
# u32 sentinel from its neighbours, `@step` adds it as a decrement, `@mask`
# keeps the high bits, and the u16 functions do the same in a word.
function @last(%x : u32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %is = cmp eq u32 %x, 4294967295
    branch %is, ^yes, ^no

  block ^yes:
    return i32 1

  block ^no:
    return i32 0
}

function @step(%x : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = binary add u32 %x, 4294967295
    return u32 %r
}

function @mask(%x : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = binary and u32 %x, 4294967280
    return u32 %r
}

function @last16(%x : u16) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %is = cmp eq u16 %x, 65535
    branch %is, ^yes, ^no

  block ^yes:
    return i32 1

  block ^no:
    return i32 0
}

function @mask16(%x : u16) -> u16 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = binary and u16 %x, 65520
    return u16 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @last(4294967295)
    %b = call i32 @last(4294967294)
    %c = call i32 @last(255)
    %d = call u32 @step(8)
    %e = call u32 @mask(4294967295)
    %f = call i32 @last16(65535)
    %g = call i32 @last16(255)
    %h = call u16 @mask16(65535)
    %ok0 = cmp eq u32 %d, 7
    %ok1 = cmp eq u32 %e, 4294967280
    %ok2 = cmp eq u16 %h, 65520
    %w0 = convert zext i32 i1 %ok0
    %w1 = convert zext i32 i1 %ok1
    %w2 = convert zext i32 i1 %ok2
    %s1 = binary add i32 %a, %b
    %s2 = binary add i32 %s1, %c
    %s3 = binary add i32 %s2, %f
    %s4 = binary add i32 %s3, %g
    %s5 = binary shl i32 %w0, 2
    %s6 = binary shl i32 %w1, 4
    %s7 = binary shl i32 %w2, 5
    %s8 = binary add i32 %s4, %s5
    %s9 = binary add i32 %s8, %s6
    %s10 = binary add i32 %s9, %s7
    return i32 %s10
}
