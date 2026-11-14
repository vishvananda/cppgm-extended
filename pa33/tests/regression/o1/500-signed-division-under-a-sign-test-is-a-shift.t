# A signed division by a power of two is a shift, and the remainder a mask,
# where a test on the way into the block shows the operand is not negative:
# the operand itself under `x > 0` or under the false side of `x < 0`, and
# one less than `x` under `x > 0`. @unknown has no test, and @other_side
# divides where the test failed: both keep the division.
function @half_if_positive(%x : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %pos = cmp gt i64 %x, 0
    branch %pos, ^yes, ^no

  block ^yes:
    %h = binary div i64 %x, 2
    return i64 %h

  block ^no:
    return i64 -1
}

function @parent_of(%hole : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %pos = cmp gt i64 %hole, 0
    branch %pos, ^yes, ^no

  block ^yes:
    %less = binary sub i64 %hole, 1
    %h = binary div i64 %less, 2
    return i64 %h

  block ^no:
    return i64 -1
}

function @low_bits(%x : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %neg = cmp lt i32 %x, 0
    branch %neg, ^no, ^yes

  block ^yes:
    %m = binary mod i32 %x, 8
    return i32 %m

  block ^no:
    return i32 -1
}

function @unknown(%x : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %h = binary div i64 %x, 2
    return i64 %h
}

function @other_side(%x : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %pos = cmp gt i64 %x, 0
    branch %pos, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    %h = binary div i64 %x, 2
    return i64 %h
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %a = call i64 @half_if_positive(7)
    %a_bad = cmp ne i64 %a, 3
    %b = call i64 @parent_of(8)
    %b_bad = cmp ne i64 %b, 3
    %c = call i32 @low_bits(29)
    %c_bad = cmp ne i32 %c, 5
    %d = call i64 @half_if_positive(0)
    %d_bad = cmp ne i64 %d, -1
    %e = call i64 @unknown(-7)
    %e_bad = cmp ne i64 %e, -3
    %f = call i64 @other_side(-7)
    %f_bad = cmp ne i64 %f, -3
    %g = call i64 @half_if_positive(-7)
    %g_bad = cmp ne i64 %g, -1
    %h = call i64 @parent_of(1)
    %h_bad = cmp ne i64 %h, 0
    %bad0 = binary or i64 %a_bad, %b_bad
    %bad1 = binary or i64 %c_bad, %d_bad
    %bad2 = binary or i64 %e_bad, %f_bad
    %bad3 = binary or i64 %g_bad, %h_bad
    %bad4 = binary or i64 %bad0, %bad1
    %bad5 = binary or i64 %bad2, %bad3
    %bad = binary or i64 %bad4, %bad5
    return i64 %bad
}
