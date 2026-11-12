# Two tests of different values with one target between them become one
# branch: the first block computes both tests and branches on their results
# joined, `a | b` where either reaches the shared target, `a & b` where
# both reach the other one, `(a | b) != 0` for two values tested against
# zero, and one sign test for two of one type: `a < 0 || b < 0` is
# `(a | b) < 0`, `a >= 0 && b >= 0` is `(a | b) >= 0` and `a < 0 && b < 0`
# is `(a & b) < 0`. The second block holds no more than its test and one
# operation that cannot trap. A first test that mostly reaches the shared
# target keeps its branch (two values seldom agree, and a value is seldom
# zero): the second test would seldom run. Two results joined cost more
# than the one branch they save where that branch is seldom mispredicted:
# where the estimate predicts the first test either way, and on a loop's
# exit test before its end. Two equalities that reach the shared target (`a == x ||
# b == y`, or `a != x && b != y` for the other one) stay apart too, and so
# does a test of a value the enclosing loop does not compute, whose branch
# is always predicted.
global @seen : i64 = 0

function @note(%v : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %new = binary add i64 %old, %v
    store i64 %new, @seen
    return void
}

# x > 3 || y < 4.
function @either(%x : i32, %y : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp gt i32 %x, 3
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp lt i32 %y, 4
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(1)
    return void

  block ^no:
    call void @note(2)
    return void
}

# x < y && p != q.
function @both(%x : i64, %y : i64, %p : ptr, %q : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp lt i64 %x, %y
    branch %a, ^more, ^no

  block ^more:
    %b = cmp ne ptr %p, %q
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(4)
    return void

  block ^no:
    call void @note(8)
    return void
}

# x >= 7 || (y & 12) == 0: the first test turns over, and the second block's
# mask goes with its test.
function @turned(%x : u32, %y : u32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp ult u32 %x, 7
    branch %a, ^more, ^yes

  block ^more:
    %m = binary and u32 %y, 12
    %b = cmp eq u32 %m, 0
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(16)
    return void

  block ^no:
    call void @note(32)
    return void
}

# x & 4 || y == 0: a test of one bit says nothing of how it mostly goes.
function @bit_first(%x : i64, %y : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and i64 %x, 4
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp eq i64 %y, 0
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(8388608)
    return void

  block ^no:
    call void @note(16777216)
    return void
}

# (x & 4) || (y & 8), two values tested against zero: (x & 4 | y & 8) != 0.
function @bits(%x : i64, %y : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and i64 %x, 4
    branch %a, ^yes, ^more

  block ^more:
    %b = binary and i64 %y, 8
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(256)
    return void

  block ^no:
    call void @note(512)
    return void
}

# x < 0 || y < 0, two sign tests: (x | y) < 0.
function @negative(%x : i32, %y : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp lt i32 %x, 0
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp lt i32 %y, 0
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(1073741824)
    return void

  block ^no:
    call void @note(2147483648)
    return void
}

# x > -1 && y >= 0: (x | y) >= 0.
function @nonnegative(%x : i64, %y : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp gt i64 %x, -1
    branch %a, ^more, ^no

  block ^more:
    %b = cmp ge i64 %y, 0
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(4294967296)
    return void

  block ^no:
    call void @note(8589934592)
    return void
}

# x < 0 && y < 0: (x & y) < 0.
function @both_negative(%x : i64, %y : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp lt i64 %x, 0
    branch %a, ^more, ^no

  block ^more:
    %b = cmp lt i64 %y, 0
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(17179869184)
    return void

  block ^no:
    call void @note(34359738368)
    return void
}

# x < 0 || y < 0 for values of two sizes: no one sign test, the two
# results joined.
function @signs_of_two_sizes(%x : i32, %y : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp lt i32 %x, 0
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp lt i64 %y, 0
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(68719476736)
    return void

  block ^no:
    call void @note(137438953472)
    return void
}

# Both branches stay: a first test the estimate predicts (x == 3 || y < 4),
# a loop's exit test (it runs while i < n && i + m < 100), first tests that
# mostly reach the shared target (two values that differ, a value that is
# not zero), two equalities that reach it (x == 3 || y == 4), two
# inequalities that reach the other target (x && y, plain values of two
# sizes), a second block that loads, one with two operations besides its
# test, a branch that states what it expects, a shared target that tells the
# two edges apart, and a loop's test of a value the loop does not compute
# (flag < 4).
function @predicted_first(%x : i32, %y : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq i32 %x, 3
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp lt i32 %y, 4
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(274877906944)
    return void

  block ^no:
    call void @note(549755813888)
    return void
}

function @exit_test(%n : i64, %m : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: 0, ^next: %step]
    %a = cmp lt i64 %i, %n
    branch %a, ^test, ^done

  block ^test:
    %s = binary add i64 %i, %m
    %b = cmp lt i64 %s, 100
    branch %b, ^next, ^done

  block ^next:
    %step = binary add i64 %i, 1
    jump ^head

  block ^done:
    return i64 %i
}

# The same two tests at the loop's end join: they end every pass, and
# joined, a pass ends in one branch, back to the header.
function @latch_test(%n : i64, %m : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: 0, ^test: %step]
    %odd = binary and i64 %i, 1
    %c = cmp ne i64 %odd, 0
    branch %c, ^odd, ^latch

  block ^odd:
    call void @note(1)
    jump ^latch

  block ^latch:
    %step = binary add i64 %i, 1
    %a = cmp lt i64 %step, %n
    branch %a, ^test, ^done

  block ^test:
    %s = binary add i64 %step, %m
    %b = cmp lt i64 %s, 100
    branch %b, ^head, ^done

  block ^done:
    return i64 %i
}

function @unequal(%x : i64, %y : i64, %p : ptr, %q : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp ne i64 %x, %y
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp ne ptr %p, %q
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(524288)
    return void

  block ^no:
    call void @note(1048576)
    return void
}

function @equal_both(%x : u32, %y : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq u32 %x, 7
    branch %a, ^more, ^no

  block ^more:
    %b = cmp eq i32 %y, 0
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(2097152)
    return void

  block ^no:
    call void @note(4194304)
    return void
}

function @two_equalities(%x : i32, %y : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq i32 %x, 3
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp eq i32 %y, 4
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(33554432)
    return void

  block ^no:
    call void @note(67108864)
    return void
}

function @two_inequalities(%x : i64, %y : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    branch %x, ^more, ^no

  block ^more:
    branch %y, ^yes, ^no

  block ^yes:
    call void @note(134217728)
    return void

  block ^no:
    call void @note(268435456)
    return void
}

function @invariant(%n : i64, %flag : u32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: 0, ^next: %step]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^done

  block ^body:
    %a = cmp ult u32 %flag, 4
    branch %a, ^hit, ^test

  block ^test:
    %b = cmp lt i64 %i, 3
    branch %b, ^hit, ^next

  block ^hit:
    call void @note(536870912)
    jump ^next

  block ^next:
    %step = binary add i64 %i, 1
    jump ^head

  block ^done:
    return void
}

function @plain(%x : i64, %y : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    branch %x, ^yes, ^more

  block ^more:
    branch %y, ^yes, ^no

  block ^yes:
    call void @note(64)
    return void

  block ^no:
    call void @note(128)
    return void
}

function @loads(%x : i32, %p : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq i32 %x, 3
    branch %a, ^yes, ^more

  block ^more:
    %v = load i32 %p
    %b = cmp eq i32 %v, 4
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(1024)
    return void

  block ^no:
    call void @note(2048)
    return void
}

function @long_test(%x : i32, %y : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq i32 %x, 3
    branch %a, ^yes, ^more

  block ^more:
    %s = binary add i32 %y, %x
    %m = binary and i32 %s, 12
    %b = cmp eq i32 %m, 4
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(4096)
    return void

  block ^no:
    call void @note(8192)
    return void
}

function @expected(%x : i32, %y : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq i32 %x, 3
    branch unlikely %a, ^yes, ^more

  block ^more:
    %b = cmp eq i32 %y, 4
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(16384)
    return void

  block ^no:
    call void @note(32768)
    return void
}

function @edges_differ(%x : i32, %y : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq i32 %x, 3
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp eq i32 %y, 4
    branch %b, ^yes, ^no

  block ^yes:
    %v = phi i64 [^entry: 65536, ^more: 131072]
    call void @note(%v)
    return void

  block ^no:
    call void @note(262144)
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    call void @either(4, 9)
    call void @either(1, 2)
    call void @either(1, 5)
    call void @negative(1, -2)
    call void @negative(1, 2)
    call void @nonnegative(0, 7)
    call void @nonnegative(5, -7)
    call void @both_negative(-1, -9)
    call void @both_negative(-1, 9)
    call void @signs_of_two_sizes(-1, 0)
    call void @signs_of_two_sizes(0, 1)
    call void @predicted_first(3, 9)
    call void @predicted_first(5, 9)
    %x1 = call i64 @exit_test(5, 0)
    %x2 = call i64 @exit_test(50, 60)
    %x3 = binary add i64 %x1, %x2
    call void @note(%x3)
    %x4 = call i64 @latch_test(5, 0)
    %x5 = call i64 @latch_test(50, 60)
    %x6 = binary add i64 %x4, %x5
    call void @note(%x6)
    call void @both(1, 2, @seen, @seen)
    call void @both(1, 2, @seen, 0)
    call void @turned(3, 4)
    call void @turned(3, 16)
    call void @turned(9, 4)
    call void @bit_first(4, 1)
    call void @bit_first(3, 1)
    call void @unequal(1, 1, @seen, @seen)
    call void @unequal(1, 1, @seen, 0)
    call void @equal_both(7, 0)
    call void @equal_both(7, 256)
    call void @plain(0, 0)
    call void @plain(0, 5)
    call void @bits(4, 0)
    call void @bits(0, 1)
    call void @two_equalities(3, 0)
    call void @two_equalities(1, 4)
    call void @two_inequalities(5, 0)
    call void @two_inequalities(5, 6)
    call void @invariant(5, 9)
    call void @loads(3, 0)
    call void @long_test(1, 3)
    call void @expected(1, 4)
    call void @edges_differ(1, 4)
    %s = load i64 @seen
    %r = binary umod i64 %s, 251
    %t = convert trunc i32 i64 %r
    return i32 %t
}
