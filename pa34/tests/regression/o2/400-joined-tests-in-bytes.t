# Two compare results joined by `or`, `and` or `xor` that only a branch
# reads are computed in their bytes: each compare sets its byte with no
# extension, the join works on the two bytes and the branch reads its flags
# (an `and` is a test of the bytes, with no result). A join that a wider
# reader also reads keeps the extended results.
function @either(%x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %a = cmp eq i64 %x, 3
    %b = cmp lt i64 %y, 4
    %j = binary or i64 %a, %b
    branch %j, ^yes, ^no

  block ^yes:
    return i64 10

  block ^no:
    return i64 20
}

function @both(%x : i32, %p : ptr, %q : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %a = cmp ugt i32 %x, 7
    %b = cmp ne ptr %p, %q
    %j = binary and i64 %a, %b
    branch %j, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @one(%x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %a = cmp eq i64 %x, 3
    %b = cmp eq i64 %y, 4
    %j = binary xor i64 %a, %b
    branch %j, ^yes, ^no

  block ^yes:
    return i64 100

  block ^no:
    return i64 200
}

function @wide(%x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %a = cmp eq i64 %x, 3
    %b = cmp lt i64 %y, 4
    %j = binary or i64 %a, %b
    branch %j, ^yes, ^no

  block ^yes:
    %r = binary add i64 %j, 1000
    return i64 %r

  block ^no:
    return i64 2000
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @either(3, 9)
    %b = call i64 @either(5, 9)
    %c = call i64 @both(8, @main, 0)
    %d = call i64 @both(8, 0, 0)
    %e = call i64 @one(3, 5)
    %f = call i64 @one(3, 4)
    %g = call i64 @wide(0, 0)
    %h = call i64 @wide(0, 9)
    %s1 = binary add i64 %a, %b
    %s2 = binary add i64 %c, %d
    %s3 = binary add i64 %e, %f
    %s4 = binary add i64 %g, %h
    %s5 = binary add i64 %s1, %s2
    %s6 = binary add i64 %s3, %s4
    %s7 = binary add i64 %s5, %s6
    %m = binary umod i64 %s7, 251
    %t = convert trunc i32 i64 %m
    return i32 %t
}
