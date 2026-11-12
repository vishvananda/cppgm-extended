# A test of one bit is zero more often than not: the bits of a flag word are
# mostly clear. Placement puts the zero side after the test and joins it
# with the rest of the path; the set side goes out of line and jumps back.

function @pick(%f : u32, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %bit = binary and u32 %f, 4
    %set = cmp ne u32 %bit, 0
    branch %set, ^marked, ^plain
  block ^marked:
    %m = binary mul i64 %a, %b
    jump ^join
  block ^plain:
    %s = binary add i64 %a, %b
    jump ^join
  block ^join:
    %r = phi i64 [^marked: %m, ^plain: %s]
    %x = binary xor i64 %r, %a
    return i64 %x
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %p = call i64 @pick(3, 5, 7)
    %q = call i64 @pick(4, 5, 7)
    %r = binary add i64 %p, %q
    %w = convert trunc i32 i64 %r
    return i32 %w
}
