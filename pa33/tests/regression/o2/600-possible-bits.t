function @quotient_of_mask(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %m = binary and i64 %x, 65535
    %n = convert trunc i32 i64 %m
    %w = convert sext i64 i32 %n
    %q = binary div i64 %w, 100000
    return i64 %q
}

function @remainder_of_mask(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %m = binary and i64 %x, 65535
    %n = convert trunc i32 i64 %m
    %w = convert sext i64 i32 %n
    %r = binary mod i64 %w, 100000
    return i64 %r
}

function @divisor_within_the_mask(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %m = binary and i64 %x, 65535
    %r = binary mod i64 %m, 60000
    return i64 %r
}

function @negative_divisor(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %m = binary and i32 %x, 15
    %r = binary div i32 %m, -100
    return i32 %r
}

function @sign_may_be_set(%x : i32) -> i64 [no_inline=yes] {
  block ^entry:
    %w = convert sext i64 i32 %x
    %m = binary and i64 %w, 4294967295
    %s = convert sext i64 i32 %x
    %q = binary div i64 %s, 8
    %r = binary add i64 %q, %m
    return i64 %r
}

function @halve_nonnegative(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %m = binary and i32 %x, 1023
    %q = binary div i32 %m, 8
    return i32 %q
}

function @rest_of_nonnegative(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %m = binary and i32 %x, 1023
    %r = binary mod i32 %m, 8
    return i32 %r
}

function @remainder_bound(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %m = binary umod i32 %x, 100
    %c = cmp lt i32 %m, 128
    %w = convert zext i32 i1 %c
    return i32 %w
}

function @signed_remainder_unbounded(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %m = binary mod i32 %x, 100
    %c = cmp ult i32 %m, 128
    %w = convert zext i32 i1 %c
    return i32 %w
}

function @compare_above_mask(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %m = binary and i64 %x, 255
    %c = cmp eq i64 %m, 256
    branch %c, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 0
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
