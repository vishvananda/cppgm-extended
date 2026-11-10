function @adjust(%p : ptr) -> ptr {
  block ^entry:
    %adjusted = index i8 %p, -8
    %zero = cmp eq ptr %p, nullptr
    %r = select ptr %zero, %p, %adjusted
    return ptr %r
}
function @adjust_predicate_first(%p : ptr) -> ptr {
  block ^entry:
    %zero = cmp eq ptr %p, nullptr
    %adjusted = index i8 %p, -8
    %r = select ptr %zero, %p, %adjusted
    return ptr %r
}
function @before_store(%p : ptr) -> i64 {
  block ^entry:
    %old = load i64 %p
    %same = cmp eq i64 %old, 5
    store i64 9, %p
    %r = select i64 %same, 11, 29
    return i64 %r
}
function @change(%p : ptr) -> i64 {
  block ^entry:
    store i64 9, %p
    return i64 0
}
function @before_call(%p : ptr) -> i64 {
  block ^entry:
    %old = load i64 %p
    %same = cmp eq i64 %old, 5
    %ignored = call i64 @change(%p)
    %r = select i64 %same, 11, 29
    return i64 %r
}
function @main() -> i64 [role=entry] {
  slot $data : obj<16x8>
  block ^entry:
    %p = index i8 $data, 8
    %expected = addr $data
    %adjusted = call ptr @adjust(%p)
    %null = call ptr @adjust(nullptr)
    %a = cmp ne ptr %adjusted, %expected
    %b = cmp ne ptr %null, nullptr
    %c = call ptr @adjust_predicate_first(%p)
    %d = call ptr @adjust_predicate_first(nullptr)
    store i64 5, %expected
    %e = call i64 @before_store(%expected)
    store i64 5, %expected
    %f = call i64 @before_call(%expected)
    %c_bad = cmp ne ptr %c, %expected
    %d_bad = cmp ne ptr %d, nullptr
    %e_bad = cmp ne i64 %e, 11
    %f_bad = cmp ne i64 %f, 11
    %ab = binary or i1 %a, %b
    %cd = binary or i1 %c_bad, %d_bad
    %ef = binary or i1 %e_bad, %f_bad
    %abcd = binary or i1 %ab, %cd
    %bad = binary or i1 %abcd, %ef
    return i64 %bad
}
