# A mask whose only reader tests zero may read memory directly, keeping
# the load's width. Signed promotion bits and intervening stores must
# still observe the original value.
function @byte_test(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load u8 %p
    %wide = convert zext i32 u8 %v
    %masked = binary and i32 %wide, 128
    %set = cmp ne i32 %masked, 0
    branch %set, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @half_test(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load u16 %p
    %wide = convert zext i32 u16 %v
    %masked = binary and i32 %wide, 32768
    %set = cmp eq i32 %masked, 0
    branch %set, ^no, ^yes
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @signed_promotion(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load i8 %p
    %wide = convert sext i32 i8 %v
    %masked = binary and i32 %wide, 256
    %set = cmp ne i32 %masked, 0
    branch %set, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @before_store(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load i32 %p
    %masked = binary and i32 %v, 16
    store i32 0, %p
    %set = cmp ne i32 %masked, 0
    branch %set, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @volatile_test(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load volatile u8 %p
    %wide = convert zext i32 u8 %v
    %masked = binary and i32 %wide, 128
    %set = cmp ne i32 %masked, 0
    branch %set, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @clear(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    store i32 0, %p
    return i32 0
}
function @before_call(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load i32 %p
    %masked = binary and i32 %v, 16
    %ignored = call i32 @clear(%p)
    %set = cmp ne i32 %masked, 0
    branch %set, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @wide_mask(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load i64 %p
    %masked = binary and i64 %v, 4294967296
    %set = cmp ne i64 %masked, 0
    branch %set, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 0
}
function @two_readers(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load i32 %p
    %masked = binary and i32 %v, 16
    %set = cmp ne i32 %masked, 0
    branch %set, ^yes, ^no
  block ^yes:
    return i32 %v
  block ^no:
    return i32 0
}
function @main() -> i64 [role=entry] {
  slot $data : i64
  block ^entry:
    %p = addr $data
    store i64 4295000208, %p
    %a = call i32 @byte_test(%p)
    %b = call i32 @half_test(%p)
    %c = call i32 @signed_promotion(%p)
    %d = call i32 @volatile_test(%p)
    %w = call i32 @wide_mask(%p)
    %full = call i32 @two_readers(%p)
    %bad_full = cmp ne i32 %full, 32912
    %e = call i32 @before_store(%p)
    store i32 32912, %p
    %f = call i32 @before_call(%p)
    %zero_byte = call i32 @byte_test(%p)
    %zero_half = call i32 @half_test(%p)
    %bad_zero = binary or i32 %zero_byte, %zero_half
    %ab = binary add i32 %a, %b
    %cd = binary add i32 %c, %d
    %ef = binary add i32 %e, %f
    %abcd = binary add i32 %ab, %cd
    %abcdef = binary add i32 %abcd, %ef
    %all = binary add i32 %abcdef, %w
    %bad_sum = cmp ne i32 %all, 7
    %bad_first = binary or i32 %bad_sum, %bad_full
    %bad = binary or i32 %bad_first, %bad_zero
    return i64 %bad
}
