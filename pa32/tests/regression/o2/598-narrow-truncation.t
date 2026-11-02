# A truncation of arithmetic on extensions of its own type is that arithmetic in
# the narrow type, so a promoted `b |= 1` is one instruction.

function @bit(%b : i8) -> i8 [no_inline=yes] {
  block ^entry:
    %w = convert sext i32 i8 %b
    %o = binary or i32 %w, 1
    %r = convert trunc i8 i32 %o
    return i8 %r
}

function @shifted(%b : i8) -> i8 [no_inline=yes] {
  block ^entry:
    %w = convert sext i32 i8 %b
    %o = binary shl i32 %w, 1
    %r = convert trunc i8 i32 %o
    return i8 %r
}

function @sum(%a : u16, %b : u16) -> u16 [no_inline=yes] {
  block ^entry:
    %x = convert zext i32 u16 %a
    %y = convert zext i32 u16 %b
    %s = binary add i32 %x, %y
    %r = convert trunc u16 i32 %s
    return u16 %r
}

# The literal keeps its low bits.
function @masked(%b : u8) -> u8 [no_inline=yes] {
  block ^entry:
    %w = convert zext i32 u8 %b
    %o = binary xor i32 %w, 511
    %r = convert trunc u8 i32 %o
    return u8 %r
}

# Each of the rest stays.
function @other_type(%b : i8) -> u8 [no_inline=yes] {
  block ^entry:
    %w = convert sext i32 i8 %b
    %o = binary or i32 %w, 1
    %r = convert trunc u8 i32 %o
    return u8 %r
}

function @shift_out(%b : i8) -> i8 [no_inline=yes] {
  block ^entry:
    %w = convert sext i32 i8 %b
    %o = binary shl i32 %w, 9
    %r = convert trunc i8 i32 %o
    return i8 %r
}

function @right_shift(%b : i8) -> i8 [no_inline=yes] {
  block ^entry:
    %w = convert sext i32 i8 %b
    %o = binary shr i32 %w, 1
    %r = convert trunc i8 i32 %o
    return i8 %r
}

function @unextended(%b : i32) -> i8 [no_inline=yes] {
  block ^entry:
    %o = binary or i32 %b, 1
    %r = convert trunc i8 i32 %o
    return i8 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
