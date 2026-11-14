# A select on one variable bit of a value, `x & (1 << k) ? a : b`, tests the
# bit with `bt x, k` and moves on its carry. The bit number's mask keeps the
# six bits `bt` reads and is not computed: bit 64 is bit 0.
function @pick(%x : i64, %n : i64) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %m = binary and i64 %n, 63
    %bit = binary shl i64 1, %m
    %and = binary and i64 %x, %bit
    %r = select i32 %and, 7, 3
    return i32 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %h = call i32 @pick(5, 2)
    %i = call i32 @pick(5, 3)
    %j = call i32 @pick(5, 64)
    %ok0 = cmp eq i32 %h, 7
    %ok1 = cmp eq i32 %i, 3
    %ok2 = cmp eq i32 %j, 7
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %r = convert trunc i32 i64 %a1
    %code = binary sub i32 1, %r
    return i32 %code
}
