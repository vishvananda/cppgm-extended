# Representation-preserving signedness copies do not prevent a masked
# condition from using one memory test. Preserve shared values and stores.
function @masked_alias(%p : ptr, %yes : i64, %no : i64) -> i64 {
  block ^entry:
    %loaded = load i32 %p
    %masked = binary and i32 %loaded, 127
    %unsigned = copy u32 %masked
    %selected = select i64 %unsigned, %yes, %no
    return i64 %selected
}
function @stored_after_mask(%p : ptr, %yes : i64, %no : i64) -> i64 {
  block ^entry:
    %loaded = load i32 %p
    %masked = binary and i32 %loaded, 127
    %unsigned = copy u32 %masked
    store i32 0, %p
    %selected = select i64 %unsigned, %yes, %no
    return i64 %selected
}
function @shared_load(%p : ptr) -> i64 {
  block ^entry:
    %loaded = load i32 %p
    %masked = binary and i32 %loaded, 127
    %unsigned = copy u32 %masked
    %selected = select i32 %unsigned, 11, 29
    %sum = binary add i32 %selected, %loaded
    %wide = convert sext i64 i32 %sum
    return i64 %wide
}
function @volatile_mask(%p : ptr, %yes : i64, %no : i64) -> i64 {
  block ^entry:
    %loaded = load volatile i32 %p
    %masked = binary and i32 %loaded, 127
    %unsigned = copy u32 %masked
    %selected = select i64 %unsigned, %yes, %no
    return i64 %selected
}
function @main() -> i64 [role=entry] {
  slot $data : i32
  block ^entry:
    %p = addr $data
    jump ^loop
  block ^loop:
    %i = phi i32 [^entry: -256, ^next: %inc]
    store i32 %i, %p
    %bits = binary and i32 %i, 127
    %nonzero = cmp ne i32 %bits, 0
    %expected = select i64 %nonzero, 11, 29
    %a = call i64 @masked_alias(%p, 11, 29)
    %b = call i64 @shared_load(%p)
    %c = call i64 @volatile_mask(%p, 11, 29)
    %d = call i64 @stored_after_mask(%p, 11, 29)
    %after = load i32 %p
    %wide_i = convert sext i64 i32 %i
    %shared_expected = binary add i64 %expected, %wide_i
    %a_bad = cmp ne i64 %a, %expected
    %b_bad = cmp ne i64 %b, %shared_expected
    %c_bad = cmp ne i64 %c, %expected
    %d_bad = cmp ne i64 %d, %expected
    %store_bad = cmp ne i32 %after, 0
    %ab = binary or i1 %a_bad, %b_bad
    %cd = binary or i1 %c_bad, %d_bad
    %abcd = binary or i1 %ab, %cd
    %bad = binary or i1 %abcd, %store_bad
    branch %bad, ^failed, ^next
  block ^next:
    %inc = binary add i32 %i, 1
    %again = cmp lt i32 %inc, 256
    branch %again, ^loop, ^done
  block ^failed:
    return i64 1
  block ^done:
    return i64 0
}
