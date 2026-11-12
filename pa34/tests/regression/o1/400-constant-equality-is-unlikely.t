# A value compared with a nonzero constant (a kind, an opcode, a character)
# seldom equals it. Placement puts the unequal side after the test and joins
# it with the rest of the path; the equal side goes out of line and jumps
# back.

function @pick(%k : i32, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %seven = cmp eq i32 %k, 7
    branch %seven, ^match, ^other
  block ^match:
    %m = binary mul i64 %a, %b
    jump ^join
  block ^other:
    %s = binary add i64 %a, %b
    jump ^join
  block ^join:
    %r = phi i64 [^match: %m, ^other: %s]
    %x = binary xor i64 %r, %a
    return i64 %x
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %p = call i64 @pick(7, 5, 7)
    %q = call i64 @pick(4, 5, 7)
    %r = binary add i64 %p, %q
    %w = convert trunc i32 i64 %r
    return i32 %w
}
