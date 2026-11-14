# A signed division by a power of two that is read only under a test of its
# operand's sign moves under the test, and is a shift there: @stored_quotient
# under `x >= 0`, @stored_remainder (a mask) under `x > 0`. In @stored_either
# the other side reads the quotient too: it stays a division.
global @out : i64 = 0

function @stored_quotient(%x : i64, %out : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %q = binary div i64 %x, 4
    %pos = cmp ge i64 %x, 0
    branch %pos, ^yes, ^no

  block ^yes:
    store i64 %q, %out
    return i64 1

  block ^no:
    return i64 0
}

function @stored_remainder(%x : i64, %out : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %m = binary mod i64 %x, 16
    %pos = cmp gt i64 %x, 0
    branch %pos, ^yes, ^no

  block ^yes:
    store i64 %m, %out
    return i64 1

  block ^no:
    return i64 0
}

function @stored_either(%x : i64, %out : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %q = binary div i64 %x, 4
    %pos = cmp ge i64 %x, 0
    branch %pos, ^yes, ^no

  block ^yes:
    store i64 %q, %out
    return i64 1

  block ^no:
    return i64 %q
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %out = addr @out
    %a0 = call i64 @stored_quotient(11, %out)
    %a = load i64 %out
    %a_bad = cmp ne i64 %a, 2
    %b0 = call i64 @stored_remainder(37, %out)
    %b = load i64 %out
    %b_bad = cmp ne i64 %b, 5
    %c0 = call i64 @stored_quotient(-11, %out)
    %c = load i64 %out
    %c_bad = cmp ne i64 %c, 5
    %d = call i64 @stored_either(-11, %out)
    %d_bad = cmp ne i64 %d, -2
    %e0 = call i64 @stored_either(11, %out)
    %e = load i64 %out
    %e_bad = cmp ne i64 %e, 2
    %bad0 = binary or i64 %a_bad, %b_bad
    %bad1 = binary or i64 %c_bad, %d_bad
    %bad2 = binary or i64 %bad0, %bad1
    %bad = binary or i64 %bad2, %e_bad
    return i64 %bad
}
