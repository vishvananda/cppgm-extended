# An extension reads the low bits of its source and extends them again, so a
# source that would extend itself need not. The byte %b shifts and gains bits
# on every pass, and only the extension after the loop reads it above eight
# bits: the loop shifts and sets bits without a `sext`, and one `sext.i8`
# after the loop extends the result. A load's extension costs nothing, so the
# extension of a loaded byte still reads it from the load.

function @pack(%p : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %j]
    %b = phi i8 [^entry: 0, ^head: %b2]
    %s = binary shl i8 %b, 1
    %a = index u8 %p, %i
    %v = load u8 %a
    %t = binary and u8 %v, 1
    %one = binary or i8 %s, 1
    %b2 = select i8 %t, %one, %s
    %j = binary add i64 %i, 1
    %more = cmp ult i64 %j, %n
    branch %more, ^head, ^out
  block ^out:
    %w = convert sext i32 i8 %b2
    %first = load i8 %p
    %f = convert sext i32 i8 %first
    %r = binary add i32 %w, %f
    return i32 %r
}

function @main() -> i32 [role=entry] {
  slot $bits : obj<8x1>

  block ^entry:
    %p = addr $bits
    store u8 1, %p
    %p1 = index u8 %p, 1
    store u8 0, %p1
    %p2 = index u8 %p, 2
    store u8 1, %p2
    %p3 = index u8 %p, 3
    store u8 1, %p3
    %p4 = index u8 %p, 4
    store u8 0, %p4
    %p5 = index u8 %p, 5
    store u8 0, %p5
    %p6 = index u8 %p, 6
    store u8 1, %p6
    %p7 = index u8 %p, 7
    store u8 1, %p7
    %r = call i32 @pack(%p, 8)
    %m = binary add i32 %r, 200
    return i32 %m
}
