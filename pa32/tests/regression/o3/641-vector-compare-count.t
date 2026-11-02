# A loop that adds up compares (count += x != 0) keeps its scalar form: a
# compare's lanes exist only as the test of a select, and here a conversion
# reads it. (Treated as a select's test, the compare had no lanes, and the
# conversion read a missing value.)
function @count(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^loop
  block ^loop:
    %s = phi i32 [^entry: 0, ^loop: %s1]
    %i = phi i64 [^entry: 0, ^loop: %i1]
    %a = index i32 %p, %i
    %v = load typed i32 %a
    %c = cmp ne i32 %v, 0
    %w = convert trunc i32 i64 %c
    %s1 = binary add nowrap i32 %w, %s
    %i1 = binary add nowrap i64 %i, 1
    %more = cmp ult i64 %i1, 256
    branch %more, ^loop, ^done
  block ^done:
    return i32 %s1
}

function @main() -> i32 [role=entry] {
  slot $a : obj<1024x16>
  block ^entry:
    %p = addr $a
    zeroinit 1024x16 %p
    %q = index i32 %p, 7
    store typed i32 3, %q
    %r = index i32 %p, 200
    store typed i32 -1, %r
    %n = call i32 @count(%p)
    return i32 %n
}
