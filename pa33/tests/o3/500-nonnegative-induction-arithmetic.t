function @nonnegative(%p : ptr, %bound : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %enter = cmp gt i64 %bound, 0
    branch %enter, ^body, ^empty
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %sum = phi i64 [^entry: 0, ^body: %total]
    %rem = binary mod i64 %i, 8
    %quot = binary div i64 %i, 4
    %q = index i64 %p, %i
    %value = load i64 %q
    %part = binary add i64 %rem, %quot
    %with_value = binary add i64 %part, %value
    %total = binary add i64 %sum, %with_value
    %next = binary add i64 %i, 1
    %more = cmp lt i64 %next, %bound
    branch %more, ^body, ^done
  block ^done:
    return i64 %total
  block ^empty:
    return i64 0
}
function @signed_range(%first : i64, %bound : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^test
  block ^test:
    %i = phi i64 [^entry: %first, ^body: %next]
    %sum = phi i64 [^entry: 0, ^body: %total]
    %more = cmp lt i64 %i, %bound
    branch %more, ^body, ^done
  block ^body:
    %rem = binary mod i64 %i, 8
    %quot = binary div i64 %i, 4
    %part = binary add i64 %rem, %quot
    %total = binary add i64 %sum, %part
    %next = binary add i64 %i, 1
    jump ^test
  block ^done:
    return i64 %sum
}
function @wrapping_counter() -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 9223372036854775807, ^body: %next]
    %rem = binary mod i64 %i, 8
    %next = binary add i64 %i, 1
    %more = cmp ge i64 %next, 0
    branch %more, ^body, ^done
  block ^done:
    return i64 %rem
}
function @main() -> i32 {
  slot $data : obj<32x8>
  block ^entry:
    zeroinit 32x8 $data
    %p = addr $data
    %a = call i64 @nonnegative(%p, 4)
    %b = call i64 @nonnegative(%p, 0)
    %c = call i64 @signed_range(-4, 0)
    %d = call i64 @wrapping_counter()
    %ab = binary add i64 %a, %b
    %cd = binary add i64 %c, %d
    %total = binary add i64 %ab, %cd
    %bad = cmp ne i64 %total, 2
    %out = convert zext i32 i1 %bad
    return i32 %out
}
