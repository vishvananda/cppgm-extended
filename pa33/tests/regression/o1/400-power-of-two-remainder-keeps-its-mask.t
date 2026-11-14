# An unsigned remainder by a power of two is one mask of its operand: it
# is not rewritten from the quotient (`x - (x / 64) * 64`), as a remainder
# by another divisor is (`@by_ten`). `@word_and_bit` is a bit set's index
# split into a word and a bit.
function @word_and_bit(%i : u32, %word : ptr) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %q = binary udiv u32 %i, 64
    store u32 %q, %word
    %r = binary umod u32 %i, 64
    return u32 %r
}

function @by_ten(%i : u32, %tens : ptr) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %q = binary udiv u32 %i, 10
    store u32 %q, %tens
    %r = binary umod u32 %i, 10
    return u32 %r
}

function @main() -> i32 [role=entry] {
  slot $q : u32

  block ^entry:
    %p = addr $q
    %a = call u32 @word_and_bit(200, %p)
    %qa = load u32 %p
    %b = call u32 @by_ten(123, %p)
    %qb = load u32 %p
    %ok0 = cmp eq u32 %a, 8
    %ok1 = cmp eq u32 %qa, 3
    %ok2 = cmp eq u32 %b, 3
    %ok3 = cmp eq u32 %qb, 12
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %a2 = binary and i64 %a1, %ok3
    %r = convert trunc i32 i64 %a2
    %code = binary sub i32 1, %r
    return i32 %code
}
