# Equal-width bit copies must not hide tagged-pointer mask identities.
function @masked(%bits : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %aligned = binary and i64 %bits, -8
    %pointer = copy ptr %aligned
    %restored = copy i64 %pointer
    %masked = binary and i64 %restored, -16
    return i64 %masked
}
function @pointer(%p : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    %bits = copy i64 %p
    %roundtrip = copy ptr %bits
    return ptr %roundtrip
}
function @main() -> i64 [role=entry] {
  slot $array : obj<16x8>
  block ^entry:
    %p = addr $array
    %result = call ptr @pointer(%p)
    %a = call i64 @masked(-1)
    %b = call i64 @masked(4294967295)
    %c = call i64 @masked(0)
    %bad_p = cmp ne ptr %result, %p
    %bad_a = cmp ne i64 %a, -16
    %bad_b = cmp ne i64 %b, 4294967280
    %bad_c = cmp ne i64 %c, 0
    %ab = binary or i64 %bad_a, %bad_b
    %cp = binary or i64 %bad_c, %bad_p
    %bad = binary or i64 %ab, %cp
    return i64 %bad
}
