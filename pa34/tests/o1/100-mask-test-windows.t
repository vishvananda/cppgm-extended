# Test only the selected bits of an ordinary load, including interior and high windows.
function @high(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load i64 %p
    %mask = binary and i64 %v, 9223372036854775808
    %b = cmp ne i64 %mask, 0
    branch %b, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @interior(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load i64 %p
    %mask = binary and i64 %v, 141836999983104
    %b = cmp ne i64 %mask, 0
    branch %b, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @wide_holes(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load i64 %p
    %mask = binary and i64 %v, 65537
    %b = cmp ne i64 %mask, 0
    branch %b, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @word_high(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load i64 %p
    %mask = binary and i64 %v, 2147483648
    %b = cmp ne i64 %mask, 0
    branch %b, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @main() -> i64 [role=entry] {
  slot $data : i64
  block ^entry:
    %p = addr $data
    store i64 -1, %p
    %a0 = call i32 @high(%p)
    %b0 = cmp ne i32 %a0, 1
    %a1 = call i32 @interior(%p)
    %b1 = cmp ne i32 %a1, 1
    %a2 = call i32 @wide_holes(%p)
    %b2 = cmp ne i32 %a2, 1
    %a3 = call i32 @word_high(%p)
    %b3 = cmp ne i32 %a3, 1
    store i64 0, %p
    %z0 = call i32 @high(%p)
    %z1 = call i32 @interior(%p)
    %z2 = call i32 @wide_holes(%p)
    %z3 = call i32 @word_high(%p)
    %r0 = binary or i32 %b0, %b1
    %r1 = binary or i32 %r0, %b2
    %r2 = binary or i32 %r1, %b3
    %r3 = binary or i32 %r2, %z0
    %r4 = binary or i32 %r3, %z1
    %r5 = binary or i32 %r4, %z2
    %r6 = binary or i32 %r5, %z3
    return i64 %r6
}
