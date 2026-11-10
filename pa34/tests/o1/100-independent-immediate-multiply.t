global @packed : i64 = 528280977415
function @narrow_memory(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %value = load i32 %p
    %product = binary mul nowrap i32 %value, 37
    %wide = convert sext i64 i32 %product
    return i64 %wide
}
# An immediate multiply can read its source independently of its result.
# Memory forms keep the original observation and immediate width.
function @imul_register(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %product = binary mul i64 %x, 37
    %result = binary sub i64 %product, %x
    return i64 %result
}
function @imul_memory(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %value = load i64 %p
    %result = binary mul i64 %value, -37
    return i64 %result
}
function @imul_word(%x : u32) -> u32 [no_inline=yes] {
  block ^entry:
    %product = binary mul u32 %x, 2654435769
    %result = binary xor u32 %product, %x
    return u32 %result
}
function @volatile_multiply(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %value = load volatile i64 %p
    %result = binary mul i64 %value, -37
    return i64 %result
}
function @before_store(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %value = load i64 %p
    store i64 0, %p
    %result = binary mul i64 %value, -37
    return i64 %result
}
function @wide_constant(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %product = binary mul i64 %x, 4294967297
    %result = binary sub i64 %product, %x
    return i64 %result
}
function @main() -> i64 [role=entry] {
  slot $cell : i64
  block ^entry:
    %cell = addr $cell
    store i64 -5, %cell
    %packed = addr @packed
    %narrow = call i64 @narrow_memory(%packed)
    %bad_narrow = cmp ne i64 %narrow, 259
    %a = call i64 @imul_register(-7)
    %b = call i64 @imul_memory(%cell)
    %v = call i64 @volatile_multiply(%cell)
    %s = call i64 @before_store(%cell)
    %wide = call i64 @wide_constant(3)
    %zero = call i64 @imul_memory(%cell)
    %c = call u32 @imul_word(4294967295)
    %bad_a = cmp ne i64 %a, -252
    %bad_b = cmp ne i64 %b, 185
    %bad_c = cmp ne u32 %c, 2654435768
    %bad = binary or i64 %bad_a, %bad_b
    %any = binary or i64 %bad, %bad_c
    %bad_v = cmp ne i64 %v, 185
    %bad_s = cmp ne i64 %s, 185
    %bad_w = cmp ne i64 %wide, 12884901888
    %bad_z = cmp ne i64 %zero, 0
    %vs = binary or i64 %bad_v, %bad_s
    %wz = binary or i64 %bad_w, %bad_z
    %extra = binary or i64 %vs, %wz
    %failed = binary or i64 %any, %extra
    %with_narrow = binary or i64 %failed, %bad_narrow
    %status = convert trunc i32 i64 %with_narrow
    return i64 %status
}
