# Unoptimized, -1 is computed rather than a constant, so a 32-bit divide could
# not see it: INT_MIN / -1 stays in the 64-bit divide and does not trap.
function @divide(%a : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %d = unary neg i32 1
    %r = binary div i32 %a, %d
    return i32 %r
}
function @main() -> i32 [role=entry] {
  block ^entry:
    %r = call i32 @divide(-2147483648)
    %bad = cmp ne i32 %r, -2147483648
    return i32 %bad
}
