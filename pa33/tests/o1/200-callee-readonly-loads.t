# Read-only facts flow from optimized callees before their callers simplify.
# A call that writes through an alias must still invalidate the earlier load.
function @read(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %value = load i64 %p
    return i64 %value
}
function @wrapper(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %value = call i64 @read(%p)
    return i64 %value
}
function @reuse(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %a = load i64 %p
    %b = call i64 @wrapper(%p)
    %c = load i64 %p
    %sum = binary add i64 %a, %b
    %out = binary add i64 %sum, %c
    return i64 %out
}
function @write(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %value = load i64 %p
    %next = binary add i64 %value, 1
    store i64 %next, %p
    return i64 %next
}
function @aliased(%p : ptr, %q : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %a = load i64 %p
    %b = call i64 @write(%q)
    %c = load i64 %p
    %sum = binary add i64 %a, %b
    %out = binary add i64 %sum, %c
    return i64 %out
}
function @main() -> i64 [role=entry] {
  slot $x : i64
  block ^entry:
    store i64 17, $x
    %p = addr $x
    %read = call i64 @reuse(%p)
    %write = call i64 @aliased(%p, %p)
    %a = cmp ne i64 %read, 51
    %b = cmp ne i64 %write, 53
    %bad = binary or i64 %a, %b
    return i64 %bad
}
