# A pointer induction from a slot's begin to a known offset of it counts its
# passes by the offsets: @fill's loop stores at 0, 8, ... 56 of $a (8 passes)
# and becomes eight stores, though its header computes the end address.
function @fill(%v : i64, %i : i64) -> i64 [no_inline=yes] {
  slot $a : obj<64x8>

  block ^entry:
    %b = addr $a
    jump ^cond

  block ^cond:
    %p = phi ptr [^entry: %b, ^body: %next]
    %base = addr $a
    %end = index i8 %base, 64
    %more = cmp ne ptr %p, %end
    branch %more, ^body, ^done

  block ^body:
    store i64 %v, %p
    %next = index i64 %p, 1
    jump ^cond

  block ^done:
    %k = binary and i64 %i, 7
    %at = index i64 %b, %k
    %x = load i64 %at
    return i64 %x
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %x = call i64 @fill(6, 13)
    %bad = cmp ne i64 %x, 6
    %r = convert zext i32 i1 %bad
    return i32 %r
}
