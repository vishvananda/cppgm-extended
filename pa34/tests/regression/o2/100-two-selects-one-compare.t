# Two selects of one compare in a block compare once: a conditional move keeps
# the flags.
function @pick(%a : i64, %b : i64, %m : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %t = cmp ule i64 %m, 1
    %x = select i64 %t, %a, %b
    %c = cmp ule i64 %m, 1
    %y = select i64 %c, 128, %a
    %r = binary xor i64 %x, %y
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %r = call i64 @pick(1, 2, 1)
    %n = convert trunc i32 i64 %r
    %z = binary sub i32 %n, 129
    return i32 %z
}
