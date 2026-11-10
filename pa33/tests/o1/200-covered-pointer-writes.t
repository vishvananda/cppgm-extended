# A later covering write makes an earlier field write unnecessary, including
# fields reached through different constant steps from one incoming pointer.
global @data : obj<16x8> = { i64 -1 i64 99 }

function @cover(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    %field = index i8 %p, 4
    store u32 17, %field
    store i64 42, %p
    return void
}

function @observed(%p : ptr, %q : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %field = index i8 %p, 4
    store u32 17, %field
    %old = load i64 %q
    store i64 42, %p
    return i64 %old
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %old = call i64 @observed(@data, @data)
    call void @cover(@data)
    %value = load i64 @data
    %next = index i64 @data, 1
    %untouched = load i64 %next
    %bad0 = cmp ne i64 %old, 77309411327
    %bad1 = cmp ne i64 %value, 42
    %bad2 = cmp ne i64 %untouched, 99
    %bad3 = binary or i64 %bad0, %bad1
    %bad = binary or i64 %bad3, %bad2
    %status = convert trunc i32 i64 %bad
    return i32 %status
}
