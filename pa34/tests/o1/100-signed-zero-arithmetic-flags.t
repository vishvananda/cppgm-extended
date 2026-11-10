# LowIR arithmetic wraps: test the result's sign even when OF was set.
function @negative32(%a : i32, %b : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %difference = binary sub i32 %a, %b
    %negative = cmp lt i32 %difference, 0
    %out = convert zext i32 i1 %negative
    return i32 %out
}
function @nonnegative64(%a : i64, %b : i64) -> i32 [no_inline=yes] {
  block ^entry:
    %sum = binary add i64 %a, %b
    %nonnegative = cmp ge i64 %sum, 0
    branch %nonnegative, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @mixed_conditions(%a : i32, %b : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %sum = binary add i32 %a, %b
    %positive = cmp gt i32 %sum, 0
    %out = convert zext i32 i1 %positive
    return i32 %out
}
function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @negative32(-2147483648, 1)
    %b = call i32 @negative32(2147483647, -1)
    %c = call i32 @nonnegative64(9223372036854775807, 1)
    %d = call i32 @nonnegative64(-9223372036854775808, -1)
    %e = call i32 @mixed_conditions(2147483647, 1)
    %f = call i32 @mixed_conditions(-2147483648, -1)
    %ab = binary add i32 %a, %b
    %cd = binary add i32 %c, %d
    %ef = binary add i32 %e, %f
    %abcd = binary add i32 %ab, %cd
    %total = binary add i32 %abcd, %ef
    %bad = cmp ne i32 %total, 3
    %result = convert zext i32 i1 %bad
    return i32 %result
}
