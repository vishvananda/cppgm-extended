# A truncation reads only the low bits of its source, so a mask that keeps
# every one of them is dropped.

function @byte(%v : u32) -> u8 [no_inline=yes] {
  block ^entry:
    %s = binary ushr u32 %v, 16
    %m = binary and u32 %s, 255
    %r = convert trunc u8 u32 %m
    return u8 %r
}

# A wider mask keeps them too.
function @wide_mask(%v : u32) -> u8 [no_inline=yes] {
  block ^entry:
    %m = binary and u32 %v, 4095
    %r = convert trunc u8 u32 %m
    return u8 %r
}

# The mask is read through a copy to the other signedness.
function @retyped(%v : i32) -> u8 [no_inline=yes] {
  block ^entry:
    %m = binary and i32 %v, 255
    %c = copy u32 %m
    %r = convert trunc u8 u32 %c
    return u8 %r
}

# A mask that clears a kept bit stays.
function @narrow_mask(%v : u32) -> u8 [no_inline=yes] {
  block ^entry:
    %m = binary and u32 %v, 127
    %r = convert trunc u8 u32 %m
    return u8 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call u8 @byte(305419896)
    %b = call u8 @wide_mask(305419896)
    %c = call u8 @retyped(-2)
    %d = call u8 @narrow_mask(255)
    %x = cmp ne u8 %a, 52
    %y = cmp ne u8 %b, 120
    %z = cmp ne u8 %c, 254
    %w = cmp ne u8 %d, 127
    %p = binary or i1 %x, %y
    %q = binary or i1 %z, %w
    %bad = binary or i1 %p, %q
    %r = convert zext i32 i1 %bad
    return i32 %r
}
