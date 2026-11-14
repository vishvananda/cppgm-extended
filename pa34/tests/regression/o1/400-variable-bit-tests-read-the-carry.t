# A test of one variable bit, `x & (1 << k)`, is `bt x, k`: the compare's
# own result reads its carry (`@word`, `@clear`), as a branch does. The
# tested word is loaded into a register, not folded into an `and`. The
# shifts and `bt` take the bit number modulo the operand's width, so a mask
# or an unsigned remainder that keeps its low six bits, read only as that
# count, is not computed: `@word` tests bit `%i` itself, and `@moved` shifts
# by `%n` (67 shifts by 3 either way).
global @bits = {
  zero 16
}

function @word(%w : ptr, %i : u32) -> u8 [binding=strong, no_inline=yes] {
  block ^entry:
    %q = binary udiv u32 %i, 64
    %at = index inbounds i64 %w, %q
    %x = load i64 %at
    %r = binary umod u32 %i, 64
    %n = convert zext i64 u32 %r
    %bit = binary shl i64 1, %n
    %and = binary and i64 %x, %bit
    %is = cmp ne i64 %and, 0
    %b = convert trunc u8 i64 %is
    return u8 %b
}

function @clear(%x : i64, %n : i64) -> u8 [binding=strong, no_inline=yes] {
  block ^entry:
    %bit = binary shl i64 1, %n
    %and = binary and i64 %bit, %x
    %is = cmp eq i64 %and, 0
    %b = convert trunc u8 i64 %is
    return u8 %b
}

function @moved(%x : i64, %n : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %m = binary and i64 %n, 127
    %r = binary shl i64 %x, %m
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %second = index inbounds i64 @bits, 1
    store i64 5, @bits
    store i64 4611686018427387904, %second
    %a = call u8 @word(@bits, 0)
    %b = call u8 @word(@bits, 1)
    %c = call u8 @word(@bits, 2)
    %d = call u8 @word(@bits, 126)
    %e = call u8 @word(@bits, 127)
    %f = call u8 @clear(5, 1)
    %g = call u8 @clear(5, 2)
    %k = call i64 @moved(5, 3)
    %l = call i64 @moved(5, 67)
    %ok0 = cmp eq u8 %a, 1
    %ok1 = cmp eq u8 %b, 0
    %ok2 = cmp eq u8 %c, 1
    %ok3 = cmp eq u8 %d, 1
    %ok4 = cmp eq u8 %e, 0
    %ok5 = cmp eq u8 %f, 1
    %ok6 = cmp eq u8 %g, 0
    %ok10 = cmp eq i64 %k, 40
    %ok11 = cmp eq i64 %l, 40
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %a2 = binary and i64 %a1, %ok3
    %a3 = binary and i64 %a2, %ok4
    %a4 = binary and i64 %a3, %ok5
    %a5 = binary and i64 %a4, %ok6
    %a9 = binary and i64 %a5, %ok10
    %a10 = binary and i64 %a9, %ok11
    %r = convert trunc i32 i64 %a10
    %code = binary sub i32 1, %r
    return i32 %code
}
