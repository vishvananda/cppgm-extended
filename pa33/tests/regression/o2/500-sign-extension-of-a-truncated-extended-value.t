# A sign extension of a truncation is the truncated value when that value
# already repeats the kept sign bit above it: here a phi of a sign-extended
# byte and a literal in the byte's range.

function @pick(%p : ptr, %c : i1) -> i32 [no_inline=yes] {
  block ^entry:
    branch %c, ^read, ^join

  block ^read:
    %b = load i8 %p
    %w = convert sext i32 i8 %b
    jump ^join

  block ^join:
    %v = phi i32 [^read: %w, ^entry: -2]
    %n = convert trunc i8 i32 %v
    %x = convert sext i32 i8 %n
    return i32 %x
}

# The extension to the other signedness is a copy of the value's bits.
function @pick_unsigned(%p : ptr, %c : i1) -> u32 [no_inline=yes] {
  block ^entry:
    branch %c, ^read, ^join

  block ^read:
    %b = load i8 %p
    %w = convert sext i32 i8 %b
    jump ^join

  block ^join:
    %v = phi i32 [^read: %w, ^entry: -2]
    %n = convert trunc i8 i32 %v
    %x = convert sext u32 i8 %n
    return u32 %x
}

# A literal outside the byte's range keeps the pair.
function @wide_literal(%p : ptr, %c : i1) -> i32 [no_inline=yes] {
  block ^entry:
    branch %c, ^read, ^join

  block ^read:
    %b = load i8 %p
    %w = convert sext i32 i8 %b
    jump ^join

  block ^join:
    %v = phi i32 [^read: %w, ^entry: 200]
    %n = convert trunc i8 i32 %v
    %x = convert sext i32 i8 %n
    return i32 %x
}

# A zero-extended byte may have bit 7 set with zeros above it: the pair stays.
function @zero_extended(%p : ptr, %c : i1) -> i32 [no_inline=yes] {
  block ^entry:
    branch %c, ^read, ^join

  block ^read:
    %b = load u8 %p
    %w = convert zext i32 u8 %b
    jump ^join

  block ^join:
    %v = phi i32 [^read: %w, ^entry: -2]
    %n = convert trunc i8 i32 %v
    %x = convert sext i32 i8 %n
    return i32 %x
}

global @byte : i8 [binding=internal] = -56

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @pick(@byte, 1)
    %b = call i32 @pick(@byte, 0)
    %c = call u32 @pick_unsigned(@byte, 1)
    %d = call i32 @wide_literal(@byte, 0)
    %e = call i32 @zero_extended(@byte, 1)
    %x = cmp ne i32 %a, -56
    %y = cmp ne i32 %b, -2
    %z = cmp ne u32 %c, 4294967240
    %w = cmp ne i32 %d, -56
    %u = cmp ne i32 %e, -56
    %p = binary or i1 %x, %y
    %q = binary or i1 %z, %w
    %s = binary or i1 %p, %q
    %bad = binary or i1 %s, %u
    %r = convert zext i32 i1 %bad
    return i32 %r
}
