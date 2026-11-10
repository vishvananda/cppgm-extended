# A sign-extended bit is minus one; it is not the 0/1 ceiling flag.
function @sext_flag(%x : i64) -> i64 [no_inline=yes] {
 block ^entry:
  %truth = cmp ne i64 %x, 0
  %bit = convert trunc i1 i64 %truth
  %flag = convert sext i64 i1 %bit
  %n = binary sub i64 %x, %flag
  %q = binary udiv i64 %n, 8
  %sum = binary add i64 %q, %flag
  %result = binary mul i64 %sum, 8
  return i64 %result
}

function @main() -> i64 [role=entry] {
 block ^entry:
  %a = call i64 @sext_flag(1)
  %b = call i64 @sext_flag(0)
  %bad_a = cmp ne i64 %a, -8
  %bad_b = cmp ne i64 %b, 0
  %bad = binary or i64 %bad_a, %bad_b
  return i64 %bad
}
