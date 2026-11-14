# Two tests joined into one branch run the second test's operation where
# the first test did not lead. Its `nowrap` held only on that path: in
# `@either` the sum may wrap when `x > 3`, so the joined form computes a
# plain `add`.
global @seen : i64 = 0

function @note(%v : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %new = binary add i64 %old, %v
    store i64 %new, @seen
    return void
}

# x > 3 || y + 1 < 4.
function @either(%x : i32, %y : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp gt i32 %x, 3
    branch %a, ^yes, ^more

  block ^more:
    %next = binary add nowrap i32 %y, 1
    %b = cmp lt i32 %next, 4
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(1)
    return void

  block ^no:
    call void @note(2)
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    call void @either(9, 2147483647)
    call void @either(0, 2)
    call void @either(0, 3)
    %seen = load i64 @seen
    %ok = cmp eq i64 %seen, 4
    %r = convert trunc i32 i64 %ok
    %code = binary sub i32 1, %r
    return i32 %code
}
