function @copy(%out : ptr [pass=indirect_result, alias=exclusive_result, object_bytes=32], %input : ptr) -> void [no_inline=yes] {
  block ^entry:
    %v0 = load typed i64 %input
    store typed i64 %v0, %out
    %from1 = index i8 %input, 8
    %to1 = index i8 %out, 8
    %v1 = load typed i64 %from1
    store typed i64 %v1, %to1
    %from2 = index i8 %input, 16
    %to2 = index i8 %out, 16
    %v2 = load typed i64 %from2
    store typed i64 %v2, %to2
    %from3 = index i8 %input, 24
    %to3 = index i8 %out, 24
    %v3 = load typed i64 %from3
    store typed i64 %v3, %to3
    return void
}
function @ordinary(%out : ptr [pass=indirect_result, object_bytes=32], %input : ptr) -> void [no_inline=yes] {
  block ^entry:
    %v0 = load typed i64 %input
    store typed i64 %v0, %out
    %from1 = index i8 %input, 8
    %to1 = index i8 %out, 8
    %v1 = load typed i64 %from1
    store typed i64 %v1, %to1
    %from2 = index i8 %input, 16
    %to2 = index i8 %out, 16
    %v2 = load typed i64 %from2
    store typed i64 %v2, %to2
    %from3 = index i8 %input, 24
    %to3 = index i8 %out, 24
    %v3 = load typed i64 %from3
    store typed i64 %v3, %to3
    return void
}
function @bounded(%out : ptr [pass=indirect_result, alias=exclusive_result, object_bytes=16], %input : ptr) -> void [no_inline=yes] {
  block ^entry:
    %v0 = load typed i64 %input
    store typed i64 %v0, %out
    %from1 = index i8 %input, 8
    %to1 = index i8 %out, 8
    %v1 = load typed i64 %from1
    store typed i64 %v1, %to1
    %from2 = index i8 %input, 16
    %to2 = index i8 %out, 16
    %v2 = load typed i64 %from2
    store typed i64 %v2, %to2
    %from3 = index i8 %input, 24
    %to3 = index i8 %out, 24
    %v3 = load typed i64 %from3
    store typed i64 %v3, %to3
    return void
}
function @main() -> i32 [role=entry] {
  slot $input : obj<40x8>
  slot $output : obj<32x8>
  block ^entry:
    %input = addr $input
    %out = addr $output
    store i64 1, %input
    %p1 = index i8 %input, 8
    %p2 = index i8 %input, 16
    %p3 = index i8 %input, 24
    store i64 2, %p1
    store i64 3, %p2
    store i64 4, %p3
    call void @copy(%out, %input)
    %last = index i8 %out, 24
    %value = load i64 %last
    %ok = cmp eq i64 %value, 4
    branch %ok, ^alias, ^bad
  block ^alias:
    call void @ordinary(%p1, %input)
    %last_alias = index i8 %input, 32
    %alias_value = load i64 %last_alias
    %alias_ok = cmp eq i64 %alias_value, 1
    branch %alias_ok, ^done, ^bad
  block ^done:
    return i32 0
  block ^bad:
    return i32 1
}
