function @shifted(%raw : i64, %base : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %x = convert trunc u32 i64 %raw
    %s = binary ushr u32 %x, 7
    %m = binary and u32 %s, 1
    %c = cmp eq u32 %m, 1
    branch %c, ^left, ^right
  block ^left:
    %a = index i64 %base, 1
    %va = load typed i64 %a
    jump ^join
  block ^right:
    %b = index i64 %base, 3
    %vb = load typed i64 %b
    jump ^join
  block ^join:
    %v = phi i64 [^left: %va, ^right: %vb]
    return i64 %v
}

function @inverted(%raw : i64, %base : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %x = convert trunc i8 i64 %raw
    %m = binary and i8 %x, -128
    %c = cmp eq i8 %m, 0
    branch %c, ^left, ^right
  block ^left:
    %a = index i16 %base, -8
    %va = load typed i64 %a
    jump ^join
  block ^right:
    %b = index i16 %base, 8
    %vb = load typed i64 %b
    jump ^join
  block ^join:
    %v = phi i64 [^left: %va, ^right: %vb]
    return i64 %v
}

function @high_bit(%raw : i64, %base : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %m = binary and i64 %raw, -9223372036854775808
    %c = cmp ne i64 %m, 0
    branch %c, ^left, ^right
  block ^left:
    %a = index i8 %base, 16
    %va = load typed i64 %a
    jump ^join
  block ^right:
    %b = index i8 %base, 48
    %vb = load typed i64 %b
    jump ^join
  block ^join:
    %v = phi i64 [^left: %va, ^right: %vb]
    return i64 %v
}

function @volatile_fields(%raw : i64, %base : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %m = binary and i64 %raw, 16
    %c = cmp ne i64 %m, 0
    branch %c, ^left, ^right
  block ^left:
    %a = index i64 %base, 1
    %va = load volatile typed i64 %a
    jump ^join
  block ^right:
    %b = index i64 %base, 3
    %vb = load volatile typed i64 %b
    jump ^join
  block ^join:
    %v = phi i64 [^left: %va, ^right: %vb]
    return i64 %v
}

function @uneven_fields(%raw : i64, %base : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %m = binary and i64 %raw, 16
    %c = cmp ne i64 %m, 0
    branch %c, ^left, ^right
  block ^left:
    %a = index i8 %base, 16
    %va = load typed i64 %a
    jump ^join
  block ^right:
    %b = index i8 %base, 40
    %vb = load typed i64 %b
    jump ^join
  block ^join:
    %v = phi i64 [^left: %va, ^right: %vb]
    return i64 %v
}
