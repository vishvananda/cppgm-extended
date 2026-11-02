# An eight-byte object returned from two four-byte stores forms in rax from
# the stored registers (or constants): a load of both stores would wait
# for neither to forward.
function @both(%a : i32, %b : i32) -> obj<8x4> [result_classes=integer, binding=strong, no_inline=yes] {
  slot $result : obj<8x4>

  block ^entry:
    %low = binary xor i32 %a, 5
    %high = binary add i32 %b, 7
    %r = addr $result
    store typed i32 %low, %r
    %h = index i8 [projection=field] %r, 4
    store typed i32 %high, %h
    return obj<8x4> $result
}

function @constant_high(%a : i32) -> obj<8x4> [result_classes=integer, binding=strong, no_inline=yes] {
  slot $result : obj<8x4>

  block ^entry:
    %low = binary xor i32 %a, 5
    %r = addr $result
    store typed i32 %low, %r
    %h = index i8 [projection=field] %r, 4
    store typed i32 9, %h
    return obj<8x4> $result
}

function @main() -> i32 [role=entry] {
  slot $x : obj<8x4>
  slot $y : obj<8x4>

  block ^entry:
    %px = addr $x
    %py = addr $y
    %vx = call obj<8x4> @both(1, 2)
    store obj<8x4> %vx, %px
    %vy = call obj<8x4> @constant_high(3)
    store obj<8x4> %vy, %py
    %x0 = load typed i32 %px
    %x4 = index i8 [projection=field] %px, 4
    %x1 = load typed i32 %x4
    %y0 = load typed i32 %py
    %y4 = index i8 [projection=field] %py, 4
    %y1 = load typed i32 %y4
    %s0 = binary add i32 %x0, %x1
    %s1 = binary add i32 %s0, %y0
    %s2 = binary add i32 %s1, %y1
    return i32 %s2
}
