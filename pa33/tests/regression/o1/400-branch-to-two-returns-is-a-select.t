# A branch to two blocks that return only chooses the returned value: the
# block computes it and returns. `@order` is a three-way compare (its inner
# choice of 1 and 0 is the compare's result), `@in_run` returns a range
# test's result, and `@scaled` runs its one arm's two instructions above the
# select. `@both_compute` keeps its branch (each side has work), and so does
# `@expected` (its branch states which side runs).
function @order(%a : u32, %b : u32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %lt = cmp ult u32 %a, %b
    branch %lt, ^less, ^rest

  block ^less:
    return i32 -1

  block ^rest:
    %gt = cmp ult u32 %b, %a
    branch %gt, ^more, ^same

  block ^more:
    return i32 1

  block ^same:
    return i32 0
}

function @in_run(%k : u16) -> u8 [binding=strong, no_inline=yes] {
  block ^entry:
    %d = binary sub u16 %k, 420
    %in = cmp ult u16 %d, 20
    branch %in, ^yes, ^no

  block ^yes:
    return u8 1

  block ^no:
    return u8 0
}

function @outside(%k : u16) -> u8 [binding=strong, no_inline=yes] {
  block ^entry:
    %d = binary sub u16 %k, 420
    %in = cmp ult u16 %d, 20
    branch %in, ^yes, ^no

  block ^yes:
    return u8 0

  block ^no:
    return u8 1
}

function @scaled(%x : i64, %y : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %big = cmp gt i64 %x, 100
    branch %big, ^plain, ^work

  block ^plain:
    return i64 %y

  block ^work:
    %m = binary mul i64 %x, 3
    %s = binary add i64 %m, %y
    return i64 %s
}

function @both_compute(%x : i64, %y : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %big = cmp gt i64 %x, 100
    branch %big, ^left, ^right

  block ^left:
    %a = binary add i64 %x, %y
    return i64 %a

  block ^right:
    %b = binary sub i64 %x, %y
    return i64 %b
}

function @expected(%x : i64, %y : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %big = cmp gt i64 %x, 100
    branch likely %big, ^plain, ^work

  block ^plain:
    return i64 %y

  block ^work:
    %m = binary mul i64 %x, 3
    return i64 %m
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %o1 = call i32 @order(3, 9)
    %o2 = call i32 @order(9, 3)
    %o3 = call i32 @order(5, 5)
    %r1 = call u8 @in_run(420)
    %r2 = call u8 @in_run(440)
    %r3 = call u8 @outside(419)
    %r4 = call u8 @outside(439)
    %s1 = call i64 @scaled(200, 7)
    %s2 = call i64 @scaled(2, 7)
    %b1 = call i64 @both_compute(200, 7)
    %e1 = call i64 @expected(2, 7)
    %ok0 = cmp eq i32 %o1, -1
    %ok1 = cmp eq i32 %o2, 1
    %ok2 = cmp eq i32 %o3, 0
    %ok3 = cmp eq u8 %r1, 1
    %ok4 = cmp eq u8 %r2, 0
    %ok5 = cmp eq u8 %r3, 1
    %ok6 = cmp eq u8 %r4, 0
    %ok7 = cmp eq i64 %s1, 7
    %ok8 = cmp eq i64 %s2, 13
    %ok9 = cmp eq i64 %b1, 207
    %ok10 = cmp eq i64 %e1, 6
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %a2 = binary and i64 %a1, %ok3
    %a3 = binary and i64 %a2, %ok4
    %a4 = binary and i64 %a3, %ok5
    %a5 = binary and i64 %a4, %ok6
    %a6 = binary and i64 %a5, %ok7
    %a7 = binary and i64 %a6, %ok8
    %a8 = binary and i64 %a7, %ok9
    %a9 = binary and i64 %a8, %ok10
    %r = convert trunc i32 i64 %a9
    %code = binary sub i32 1, %r
    return i32 %code
}
