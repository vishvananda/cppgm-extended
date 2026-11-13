# A table of byte results holds `i8` elements. A `u8` phi reads the element's
# bits as they are: the load takes the phi's type, and no conversion between
# two types of one width is formed.
function @pick(%k : u8, %other : u8) -> u8 [no_inline=yes] {
  block ^entry:
    %wide = convert zext i32 u8 %k
    switch %wide, ^merge, 2:^case2, 3:^case3, 4:^case4, 5:^case5, 6:^case6, 7:^case7

  block ^case2:
    jump ^merge

  block ^case3:
    jump ^merge

  block ^case4:
    jump ^merge

  block ^case5:
    jump ^merge

  block ^case6:
    jump ^merge

  block ^case7:
    jump ^merge

  block ^merge:
    %result = phi u8 [^entry: %other, ^case2: 7, ^case3: 9, ^case4: 12, ^case5: 3, ^case6: 40, ^case7: 41]
    %mixed = binary xor u8 %result, %other
    return u8 %mixed
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call u8 @pick(4, 1)
    %b = call u8 @pick(7, 2)
    %c = call u8 @pick(9, 3)
    %wa = convert zext i32 u8 %a
    %wb = convert zext i32 u8 %b
    %wc = convert zext i32 u8 %c
    %ab = binary add i32 %wa, %wb
    %sum = binary add i32 %ab, %wc
    return i32 %sum
}
