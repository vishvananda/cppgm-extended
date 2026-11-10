# An integer-class return carries the object's bytes; the rest of its ABI
# register is padding and need not be reloaded from a wider frame location.
function @byte_return(%p : ptr) -> obj<1x1> [result_classes=integer, no_inline=yes] {
  slot $result : obj<1x1>
  block ^entry:
    %to = addr $result
    %value = load u8 %p
    store u8 %value, %to
    return obj<1x1> $result
}
function @half_return(%p : ptr) -> obj<2x2> [result_classes=integer, no_inline=yes] {
  slot $result : obj<2x2>
  block ^entry:
    %to = addr $result
    %value = load u16 %p
    store u16 %value, %to
    return obj<2x2> $result
}
function @tail_return(%p : ptr) -> obj<9x1> [result_classes=integer_integer, no_inline=yes] {
  slot $result : obj<9x1>
  block ^entry:
    %to = addr $result
    %a = load i64 %p
    %from_tail = index i8 %p, 8
    %b = load u8 %from_tail
    store i64 %a, %to
    %to_tail = index i8 %to, 8
    store u8 %b, %to_tail
    return obj<9x1> $result
}
function @main() -> i64 [role=entry] {
  slot $input : obj<16x8>
  slot $byte : obj<1x1>
  slot $half : obj<2x2>
  slot $tail : obj<9x1>
  block ^entry:
    %p = addr $input
    store i64 -1, %p
    %p8 = index i8 %p, 8
    store u8 137, %p8
    %byte = call obj<1x1> @byte_return(%p)
    %half = call obj<2x2> @half_return(%p)
    %tail = call obj<9x1> @tail_return(%p)
    %b = addr $byte
    %h = addr $half
    %t = addr $tail
    copyobj 1x1 %byte, %b
    copyobj 2x2 %half, %h
    copyobj 9x1 %tail, %t
    %bv = load u8 %b
    %hv = load u16 %h
    %tv = load i64 %t
    %t8 = index i8 %t, 8
    %tt = load u8 %t8
    %bad_b = cmp ne u8 %bv, 255
    %bad_h = cmp ne u16 %hv, 65535
    %bad_t = cmp ne i64 %tv, -1
    %bad_tt = cmp ne u8 %tt, 137
    %first = binary or i64 %bad_b, %bad_h
    %second = binary or i64 %bad_t, %bad_tt
    %bad = binary or i64 %first, %second
    return i64 %bad
}
