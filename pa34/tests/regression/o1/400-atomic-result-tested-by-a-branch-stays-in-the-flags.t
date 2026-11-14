# An atomic operation whose result only the branch ending its block tests
# leaves it in the flags, with no result register. `@acquire` jumps on the
# zero flag of its compare-exchange, whose expected value is the literal the
# store before it wrote (no load of the slot); `@retry` reaches its branch
# through a compare with zero. `@release` jumps on the zero flag of a
# `lock_add`, and `@count`, whose sum nothing reads, is a `lock_add` alone.
# `@both` reads its sum twice and keeps `lock_xadd`.
function @acquire(%lock : ptr) -> i32 [binding=strong, no_inline=yes] {
  slot $expected : u8
  block ^entry:
    store u8 0, $expected
    %e = addr $expected
    %ok = atomic_compare_exchange u8 %lock, %e, 1, 5, 5
    %c = convert trunc u8 i64 %ok
    branch %c, ^taken, ^busy
  block ^taken:
    return i32 1
  block ^busy:
    return i32 0
}

function @retry(%cell : ptr, %add : i64) -> i64 [binding=strong, no_inline=yes] {
  slot $old : i64
  block ^entry:
    %o = addr $old
    %first = atomic_load i64 %cell, 5
    store i64 %first, $old
    jump ^attempt
  block ^attempt:
    %seen = load i64 $old
    %next = binary add i64 %seen, %add
    %ok = atomic_compare_exchange i64 %cell, %o, %next, 5, 5
    %c = convert trunc u8 i64 %ok
    %failed = cmp eq u8 %c, 0
    branch %failed, ^attempt, ^done
  block ^done:
    %r = load i64 $old
    return i64 %r
}

function @release(%count : ptr) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %left = atomic_add_fetch i32 %count, -1, 5
    %last = cmp eq i32 %left, 0
    branch %last, ^drop, ^keep
  block ^drop:
    return i32 1
  block ^keep:
    return i32 0
}

function @count(%count : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %ignored = atomic_add_fetch i32 %count, 3, 5
    return void
}

function @both(%count : ptr) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %sum = atomic_add_fetch i32 %count, 4, 5
    %zero = cmp eq i32 %sum, 0
    branch %zero, ^none, ^some
  block ^none:
    return i32 -1
  block ^some:
    return i32 %sum
}

function @main() -> i32 [role=entry] {
  slot $lock : u8
  slot $cell : i64
  slot $count : i32
  block ^entry:
    %l = addr $lock
    %c = addr $cell
    %n = addr $count
    store u8 0, %l
    store i64 5, %c
    store i32 2, %n
    %a = call i32 @acquire(%l)
    %b = call i32 @acquire(%l)
    %held = load u8 %l
    %old = call i64 @retry(%c, 7)
    %now = load i64 %c
    %d = call i32 @release(%n)
    %e = call i32 @release(%n)
    call void @count(%n)
    %f = load i32 %n
    %g = call i32 @both(%n)
    %ok0 = cmp eq i32 %a, 1
    %ok1 = cmp eq i32 %b, 0
    %ok2 = cmp eq u8 %held, 1
    %ok3 = cmp eq i64 %old, 5
    %ok4 = cmp eq i64 %now, 12
    %ok5 = cmp eq i32 %d, 0
    %ok6 = cmp eq i32 %e, 1
    %ok7 = cmp eq i32 %f, 3
    %ok8 = cmp eq i32 %g, 7
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %a2 = binary and i64 %a1, %ok3
    %a3 = binary and i64 %a2, %ok4
    %a4 = binary and i64 %a3, %ok5
    %a5 = binary and i64 %a4, %ok6
    %a6 = binary and i64 %a5, %ok7
    %a7 = binary and i64 %a6, %ok8
    %r = convert trunc i32 i64 %a7
    %code = binary sub i32 1, %r
    return i32 %code
}
