# Run before and after optimization: a loop's exit values at a runtime bound,
# from before the bound, at it and past it, where the narrow sum wraps too.
function @count_up(%from : i32, %to : i32) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: %from, ^body: %next]
    %sum = phi i32 [^entry: 7, ^body: %added]
    %more = cmp lt i32 %i, %to
    branch %more, ^body, ^done

  block ^body:
    %added = binary add i32 %sum, 3
    %next = binary add i32 %i, 1
    jump ^loop

  block ^done:
    %scaled = binary mul i32 %i, 100
    %r = binary add i32 %scaled, %sum
    return i32 %r
}

function @narrow(%n : i32) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^body: %next]
    %sum = phi i8 [^entry: 0, ^body: %cut]
    %more = cmp lt i32 %i, %n
    branch %more, ^body, ^done

  block ^body:
    %wide = convert sext i32 i8 %sum
    %added = binary add nowrap i32 %wide, 3
    %cut = convert trunc i8 i32 %added
    %next = binary add nowrap i32 %i, 1
    jump ^loop

  block ^done:
    %r = convert sext i32 i8 %sum
    return i32 %r
}

function @count_down(%from : i32, %to : i32) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: %from, ^body: %next]
    %sum = phi i32 [^entry: 0, ^body: %added]
    %more = cmp gt i32 %i, %to
    branch %more, ^body, ^done

  block ^body:
    %added = binary add i32 %sum, 6
    %next = binary add i32 %i, -1
    jump ^loop

  block ^done:
    return i32 %sum
}

function @main() -> i64 [role=entry] {
  block ^entry:
    jump ^c0

  block ^c0:
    %r0 = call i32 @count_up(0, 5)
    %ok0 = cmp eq i32 %r0, 522
    branch %ok0, ^c1, ^bad0

  block ^bad0:
    return i64 1

  block ^c1:
    %r1 = call i32 @count_up(5, 0)
    %ok1 = cmp eq i32 %r1, 507
    branch %ok1, ^c2, ^bad1

  block ^bad1:
    return i64 2

  block ^c2:
    %r2 = call i32 @count_up(0, 0)
    %ok2 = cmp eq i32 %r2, 7
    branch %ok2, ^c3, ^bad2

  block ^bad2:
    return i64 3

  block ^c3:
    %r3 = call i32 @count_up(-3, 2)
    %ok3 = cmp eq i32 %r3, 222
    branch %ok3, ^c4, ^bad3

  block ^bad3:
    return i64 4

  block ^c4:
    %r4 = call i32 @count_up(2147483646, 2147483647)
    %ok4 = cmp eq i32 %r4, -90
    branch %ok4, ^c5, ^bad4

  block ^bad4:
    return i64 5

  block ^c5:
    %r5 = call i32 @narrow(100)
    %ok5 = cmp eq i32 %r5, 44
    branch %ok5, ^c6, ^bad5

  block ^bad5:
    return i64 6

  block ^c6:
    %r6 = call i32 @narrow(0)
    %ok6 = cmp eq i32 %r6, 0
    branch %ok6, ^c7, ^bad6

  block ^bad6:
    return i64 7

  block ^c7:
    %r7 = call i32 @narrow(-1)
    %ok7 = cmp eq i32 %r7, 0
    branch %ok7, ^c8, ^bad7

  block ^bad7:
    return i64 8

  block ^c8:
    %r8 = call i32 @narrow(43)
    %ok8 = cmp eq i32 %r8, -127
    branch %ok8, ^c9, ^bad8

  block ^bad8:
    return i64 9

  block ^c9:
    %r9 = call i32 @count_down(5, 0)
    %ok9 = cmp eq i32 %r9, 30
    branch %ok9, ^c10, ^bad9

  block ^bad9:
    return i64 10

  block ^c10:
    %r10 = call i32 @count_down(0, 5)
    %ok10 = cmp eq i32 %r10, 0
    branch %ok10, ^pass, ^bad10

  block ^bad10:
    return i64 11

  block ^pass:
    return i64 0
}
