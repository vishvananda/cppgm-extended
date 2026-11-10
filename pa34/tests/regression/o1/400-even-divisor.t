# 1000000000 by multiplication needs 65 bits for a full dividend. Its factor of
# 512 shifted out of the dividend first leaves a multiplier that fits.
function @billions(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %q = binary udiv i64 %x, 1000000000
    return i64 %q
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @billions(18446744073709551615)
    %b = call i64 @billions(999999999)
    %c = call i64 @billions(1000000000)
    %x = binary add i64 %b, %c
    %t = binary udiv i64 %a, 100000000
    %r = binary add i64 %t, %x
    %n = convert trunc i32 i64 %r
    return i32 %n
}
