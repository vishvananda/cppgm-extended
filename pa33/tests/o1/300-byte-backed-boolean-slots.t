function @boolean_bytes(%x : i32) -> i32 [no_inline=yes] {
  slot $a : i1
  slot $b : i1
  block ^entry:
    %positive = cmp gt i32 %x, 0
    store i1 %positive, $a
    %first = load i8 $a
    branch %first, ^yes, ^no
  block ^yes:
    store i1 %first, $b
    jump ^done
  block ^no:
    store i1 0, $b
    jump ^done
  block ^done:
    %second = load u8 $b
    %out = convert zext i32 u8 %second
    return i32 %out
}
function @ordinary_byte(%x : u8) -> i32 [no_inline=yes] {
  slot $byte : i1
  block ^entry:
    store u8 %x, $byte
    %read = load u8 $byte
    %out = convert zext i32 u8 %read
    return i32 %out
}
function @volatile_byte(%x : i32) -> i32 [no_inline=yes] {
  slot $flag : i1
  block ^entry:
    %bit = cmp ne i32 %x, 0
    store volatile i1 %bit, $flag
    %read = load volatile u8 $flag
    %out = convert zext i32 u8 %read
    return i32 %out
}
function @set_byte(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    store i8 -128, %p
    return void
}
function @escaped_byte() -> i32 [no_inline=yes] {
  slot $flag : i1
  block ^entry:
    store i1 1, $flag
    %p = addr $flag
    call void @set_byte(%p)
    %read = load i8 $flag
    %out = convert sext i32 i8 %read
    return i32 %out
}
function @main() -> i32 {
  block ^entry:
    %a = call i32 @boolean_bytes(17)
    %b = call i32 @boolean_bytes(-17)
    %c = call i32 @ordinary_byte(128)
    %d = call i32 @volatile_byte(1)
    %e = call i32 @escaped_byte()
    %ab = binary add i32 %a, %b
    %cd = binary add i32 %c, %d
    %abcd = binary add i32 %ab, %cd
    %total = binary add i32 %abcd, %e
    %bad = cmp ne i32 %total, 2
    %out = convert zext i32 i1 %bad
    return i32 %out
}
