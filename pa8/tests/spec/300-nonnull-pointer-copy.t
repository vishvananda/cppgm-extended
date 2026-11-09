global @cell : i64 = 7
function @refine(%p : ptr) -> ptr {
  block ^entry:
    %q = copy nonnull ptr %p
    return ptr %q
}
function @main() -> i64 {
  block ^entry:
    %p = addr @cell
    %q = call ptr @refine(%p)
    %v = load i64 %q
    return i64 %v
}
