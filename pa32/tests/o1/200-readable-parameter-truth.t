# Readable storage is non-null; an unannotated pointer remains nullable.
function @marked_eq(%p : ptr [object_bytes=8]) -> i64 [no_inline=yes] {
  block ^entry:
    %zero = cmp eq ptr %p, 0
    return i64 %zero
}
function @marked_branch(%p : ptr [pass=by_address, object_bytes=8]) -> i64 [no_inline=yes] {
  block ^entry:
    branch %p, ^present, ^missing
  block ^present:
    %v = load i64 %p
    return i64 %v
  block ^missing:
    return i64 -99
}
function @marked_select(%p : ptr [object_bytes=8], %other : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    %r = select ptr %p, %p, %other
    return ptr %r
}
function @unmarked(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %zero = cmp eq ptr %p, 0
    return i64 %zero
}
function @main() -> i64 [role=entry] {
  slot $value : i64
  block ^entry:
    %p = addr $value
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: -512, ^next: %inext]
    store i64 %i, %p
    %a = call i64 @marked_eq(%p)
    %b = call i64 @marked_branch(%p)
    %c = call ptr @marked_select(%p, 0)
    %d = call i64 @unmarked(0)
    %e = call i64 @unmarked(%p)
    %bad_b = cmp ne i64 %b, %i
    %bad_c = cmp ne ptr %c, %p
    %bad_d = cmp ne i64 %d, 1
    %ab = binary or i64 %a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %abcd = binary or i64 %ab, %cd
    %bad = binary or i64 %abcd, %e
    branch %bad, ^fail, ^next
  block ^next:
    %inext = binary add i64 %i, 1
    %more = cmp ne i64 %inext, 512
    branch %more, ^loop, ^done
  block ^fail:
    return i64 1
  block ^done:
    return i64 0
}
