# A select whose condition is an `and` it alone reads tests the operands
# (test x, y) before the cmov and computes no result for the and. An and
# that another instruction also reads keeps its register.
function @pick(%x : i64, %y : i64, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %m = binary and i64 %x, %y
    %r = select i64 %m, %a, %b
    return i64 %r
}

function @bit(%x : i32, %a : i32, %b : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %m = binary and i32 %x, 1024
    %r = select i32 %m, %a, %b
    return i32 %r
}

function @shared(%x : i64, %y : i64, %a : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %m = binary and i64 %x, %y
    %r = select i64 %m, %a, 7
    %s = binary add i64 %r, %m
    return i64 %s
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @pick(6, 3, 10, 20)
    %b = call i64 @pick(4, 3, 10, 20)
    %c = call i32 @bit(1024, 1, 2)
    %d = call i32 @bit(1023, 1, 2)
    %e = call i64 @shared(6, 3, 100)
    %f = binary add i64 %a, %b
    %g = binary add i64 %f, %e
    %h = convert trunc i32 i64 %g
    %i = binary mul i32 %c, 10
    %j = binary add i32 %i, %d
    %k = binary add i32 %h, %j
    return i32 %k
}
