# An index whose offset is a word step that never wraps (`nowrap`) reads the
# step's operand, extended once, and takes the constant in its displacement:
# `@pair` loads p[i + 1] and p[i + 2] and `@before` loads p[i - 1] with no
# add or subtract. A step without the fact may wrap at 32 bits, so
# `@wrapping` computes it.
function @pair(%p : ptr, %i : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary add nowrap i32 %i, 1
    %pa = index i32 %p, %a
    %va = load i32 %pa
    %b = binary add nowrap i32 %i, 2
    %pb = index i32 %p, %b
    %vb = load i32 %pb
    %s = binary add i32 %va, %vb
    return i32 %s
}

function @before(%p : ptr, %i : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary sub nowrap i32 %i, 1
    %pa = index i32 %p, %a
    %va = load i32 %pa
    return i32 %va
}

function @wrapping(%p : ptr, %i : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary add i32 %i, 1
    %pa = index i32 %p, %a
    %va = load i32 %pa
    return i32 %va
}

function @main() -> i32 [role=entry] {
  slot $data : obj<16x4>

  block ^entry:
    %buf = addr $data
    store i32 1, %buf
    %b1 = index i32 %buf, 1
    store i32 2, %b1
    %b2 = index i32 %buf, 2
    store i32 4, %b2
    %b3 = index i32 %buf, 3
    store i32 8, %b3
    %x = call i32 @pair(%buf, 1)
    %y = call i32 @before(%buf, 1)
    %z = call i32 @wrapping(%buf, 0)
    %xy = binary add i32 %x, %y
    %t = binary add i32 %xy, %z
    return i32 %t
}
