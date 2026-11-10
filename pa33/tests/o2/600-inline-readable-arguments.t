# A readable formal is non-null only where that call executes. Preserve the
# fact through a required inline copy, without refining other paths' input.
function @late_value(%p : ptr) -> i64 {
  block ^entry:
    branch %p, ^present, ^missing
  block ^present:
    %v = load i64 %p
    return i64 %v
  block ^missing:
    return i64 -99
}
function @readable_wrapper(%p : ptr [object_bytes=8]) -> i64 [force_inline=yes] {
  block ^entry:
    %v = call i64 @late_value(%p)
    return i64 %v
}
function @path(%p : ptr, %active : i64) -> i64 [no_inline=yes] {
  block ^entry:
    branch %active, ^yes, ^no
  block ^yes:
    %v = call i64 @readable_wrapper(%p)
    return i64 %v
  block ^no:
    %missing = cmp eq ptr %p, 0
    return i64 %missing
}
function @zero_trip(%p : ptr, %count : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^body: %inext]
    %sum = phi i64 [^entry: 0, ^body: %sum_next]
    %more = cmp ult i64 %i, %count
    branch %more, ^body, ^done
  block ^body:
    %q = copy nonnull ptr %p
    %v = load i64 %q
    %sum_next = binary add i64 %sum, %v
    %inext = binary add i64 %i, 1
    jump ^header
  block ^done:
    %missing = cmp eq ptr %p, 0
    %result = binary add i64 %sum, %missing
    return i64 %result
}
function @choice(%p : ptr, %take : i64) -> i64 [no_inline=yes] {
  block ^entry:
    branch %take, ^yes, ^no
  block ^yes:
    %q = copy nonnull ptr %p
    jump ^merge
  block ^no:
    jump ^merge
  block ^merge:
    %chosen = phi ptr [^yes: %q, ^no: 0]
    %missing_input = cmp eq ptr %p, 0
    %missing_result = cmp eq ptr %chosen, 0
    %r = binary add i64 %missing_input, %missing_result
    return i64 %r
}
function @main() -> i64 [role=entry] {
  slot $value : i64
  block ^entry:
    %p = addr $value
    store i64 7, $value
    %a = call i64 @path(%p, 1)
    %b = call i64 @path(%p, 0)
    %c = call i64 @path(0, 0)
    %z0 = call i64 @zero_trip(0, 0)
    %z3 = call i64 @zero_trip(%p, 3)
    %c0 = call i64 @choice(0, 0)
    %c1 = call i64 @choice(%p, 1)
    %bad_z0 = cmp ne i64 %z0, 1
    %bad_z3 = cmp ne i64 %z3, 21
    %bad_c0 = cmp ne i64 %c0, 2
    %zs = binary or i64 %bad_z0, %bad_z3
    %cs = binary or i64 %bad_c0, %c1
    %guards = binary or i64 %zs, %cs
    %bad_a = cmp ne i64 %a, 7
    %bad_c = cmp ne i64 %c, 1
    %ab = binary or i64 %bad_a, %b
    %abc = binary or i64 %ab, %bad_c
    %bad = binary or i64 %abc, %guards
    return i64 %bad
}
