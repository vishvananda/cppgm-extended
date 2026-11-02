# A remainder next to the quotient of the same operands reads the quotient: one
# divide, whichever comes first. Other operands keep their own divide.
function @after(%x : i64, %out : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %q = binary udiv i64 %x, 1000
    %r = binary umod i64 %x, 1000
    store typed i64 %r, %out
    return i64 %q
}

function @before(%x : i32, %y : i32, %out : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %r = binary mod i32 %x, %y
    %q = binary div i32 %x, %y
    store typed i32 %q, %out
    return i32 %r
}

function @other(%x : i32, %y : i32, %z : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %q = binary div i32 %x, %y
    %r = binary mod i32 %x, %z
    %s = binary add i32 %q, %r
    return i32 %s
}

function @main() -> i32 [role=entry] {
  slot $a : obj<8x8>
  block ^entry:
    %p = addr $a
    %q = call i64 @after(12345, %p)
    %r = load typed i64 %p
    %n = convert trunc i32 i64 %q
    %m = convert trunc i32 i64 %r
    %s = call i32 @before(-17, 5, %p)
    %t = load typed i32 %p
    %u = call i32 @other(30, 7, 4)
    %a1 = binary add i32 %n, %m
    %a2 = binary mul i32 %s, 10
    %a3 = binary add i32 %a1, %a2
    %a4 = binary add i32 %a3, %t
    %a5 = binary sub i32 %a4, %u
    %c = cmp eq i32 %a5, 328
    branch %c, ^ok, ^bad

  block ^ok:
    return i32 0

  block ^bad:
    return i32 1
}
