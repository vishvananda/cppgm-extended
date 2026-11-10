function @round_trip(%x : f32) -> f32 [binding=strong, no_inline=yes] {
  slot $first : obj<4x4>
  slot $second : obj<4x4>
  block ^entry:
    %a = addr $first
    store typed f32 %x, %a
    %b = addr $second
    copyobj 4x4 $first, %b
    %bits = load u32 %b
    %c = addr $first
    store u32 %bits, %c
    %y = load f32 %c
    return f32 %y
}

function @double_bits(%x : f64) -> i64 [binding=strong, no_inline=yes] {
  slot $s : obj<8x8>
  block ^entry:
    %a = addr $s
    store typed f64 %x, %a
    %bits = load i64 %a
    return i64 %bits
}

function @float_of_bits(%x : i32) -> f32 [binding=strong, no_inline=yes] {
  slot $s : obj<4x4>
  block ^entry:
    %a = addr $s
    store i32 %x, %a
    %y = load f32 %a
    return f32 %y
}

function @merged(%c : i1, %x : f32, %y : u32) -> f32 [binding=strong, no_inline=yes] {
  slot $s : obj<4x4>
  block ^entry:
    %a = addr $s
    branch %c, ^floating, ^bits

  block ^floating:
    store f32 %x, %a
    jump ^join

  block ^bits:
    store u32 %y, %a
    jump ^join

  block ^join:
    %r = load f32 %a
    return f32 %r
}

function @literal_bits() -> u32 [binding=strong, no_inline=yes] {
  slot $s : obj<4x4>
  block ^entry:
    %a = addr $s
    store f32 1.5f, %a
    %bits = load u32 %a
    return u32 %bits
}

function @literal_float() -> f32 [binding=strong, no_inline=yes] {
  slot $s : obj<4x4>
  block ^entry:
    %a = addr $s
    store u32 1065353216, %a
    %y = load f32 %a
    return f32 %y
}

function @half_of_bits(%x : f32) -> u16 [binding=strong, no_inline=yes] {
  slot $s : obj<4x4>
  block ^entry:
    %a = addr $s
    store f32 %x, %a
    %low = load u16 %a
    return u16 %low
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
