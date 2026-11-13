# A remainder before a branch and the quotient after the branch's join share
# one divide: every path from the remainder reaches the quotient, so the
# quotient moves up to the remainder, which becomes x - (x / y) * y. The
# divisor is converted again before the quotient, as the C++ lowering writes
# it, so the two divisions match through equal conversions. The other two
# functions keep both divisions: one path returns before the quotient, or the
# join is reached without the remainder.
function @digits(%n : i64, %base : i32, %end : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    jump ^body

  block ^body:
    %v = phi i64 [^entry: %n, ^join: %q]
    %at = phi ptr [^entry: %end, ^join: %before]
    %wide = convert sext i64 i32 %base
    %r = binary mod i64 %v, %wide
    %before = index inbounds i8 %at, -1
    %small = cmp lt i64 %r, 10
    branch %small, ^digit, ^letter

  block ^digit:
    %d = binary add i64 %r, 48
    %db = convert trunc i8 i64 %d
    store i8 %db, %before
    jump ^join

  block ^letter:
    %l = binary add i64 %r, 87
    %lb = convert trunc i8 i64 %l
    store i8 %lb, %before
    jump ^join

  block ^join:
    %again = convert sext i64 i32 %base
    %q = binary div i64 %v, %again
    %more = cmp ne i64 %q, 0
    branch %more, ^body, ^done

  block ^done:
    return ptr %before
}

function @digits_until_letter(%n : i64, %base : i32, %end : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    jump ^body

  block ^body:
    %v = phi i64 [^entry: %n, ^digit: %q]
    %at = phi ptr [^entry: %end, ^digit: %before]
    %wide = convert sext i64 i32 %base
    %r = binary mod i64 %v, %wide
    %before = index inbounds i8 %at, -1
    %small = cmp lt i64 %r, 10
    branch %small, ^digit, ^letter

  block ^letter:
    return ptr %at

  block ^digit:
    %d = binary add i64 %r, 48
    %db = convert trunc i8 i64 %d
    store i8 %db, %before
    %again = convert sext i64 i32 %base
    %q = binary div i64 %v, %again
    %more = cmp ne i64 %q, 0
    branch %more, ^body, ^done

  block ^done:
    return ptr %before
}

function @remainder_on_one_path(%n : i64, %d : i64, %first : i64) -> i64 [no_inline=yes] {
  block ^entry:
    branch %first, ^remainder, ^join

  block ^remainder:
    %r = binary mod i64 %n, %d
    %scaled = binary mul i64 %r, 1000
    jump ^join

  block ^join:
    %low = phi i64 [^entry: 0, ^remainder: %scaled]
    %q = binary div i64 %n, %d
    %sum = binary add i64 %low, %q
    return i64 %sum
}

function @main() -> i64 [role=entry] {
  slot $buffer : obj<32x8>
  block ^entry:
    %start = addr $buffer
    %end = index inbounds i8 %start, 32
    %hex = call ptr @digits(48879, 16, %end)
    %want_hex = index inbounds i8 %start, 28
    %bad_hex = cmp ne ptr %hex, %want_hex
    %b = load i8 %hex
    %bad_b = cmp ne i8 %b, 98
    %f_at = index inbounds i8 %hex, 3
    %f = load i8 %f_at
    %bad_f = cmp ne i8 %f, 102
    %neg = call ptr @digits(-255, 16, %hex)
    %want_neg = index inbounds i8 %start, 26
    %bad_neg = cmp ne ptr %neg, %want_neg
    %bang = load i8 %neg
    %bad_bang = cmp ne i8 %bang, 33
    %stop = call ptr @digits_until_letter(419, 16, %neg)
    %want_stop = index inbounds i8 %start, 25
    %bad_stop = cmp ne ptr %stop, %want_stop
    %three = load i8 %stop
    %bad_three = cmp ne i8 %three, 51
    %down = call ptr @digits_until_letter(-1234, 10, %stop)
    %want_down = index inbounds i8 %start, 21
    %bad_down = cmp ne ptr %down, %want_down
    %slash = load i8 %down
    %bad_slash = cmp ne i8 %slash, 47
    %one = call i64 @remainder_on_one_path(-7, 2, 1)
    %bad_one = cmp ne i64 %one, -1003
    %other = call i64 @remainder_on_one_path(-7, 2, 0)
    %bad_other = cmp ne i64 %other, -3
    %a1 = binary or i1 %bad_hex, %bad_b
    %a2 = binary or i1 %a1, %bad_f
    %a3 = binary or i1 %a2, %bad_neg
    %a4 = binary or i1 %a3, %bad_bang
    %a5 = binary or i1 %a4, %bad_stop
    %a6 = binary or i1 %a5, %bad_three
    %a7 = binary or i1 %a6, %bad_down
    %a8 = binary or i1 %a7, %bad_slash
    %a9 = binary or i1 %a8, %bad_one
    %a10 = binary or i1 %a9, %bad_other
    %out = convert zext i64 i1 %a10
    return i64 %out
}
