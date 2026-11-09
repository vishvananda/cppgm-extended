function @raw_signed() -> i8 {
  block ^entry:
    %bits = const i64 4294967425
    return i8 %bits
}
function @raw_unsigned() -> u8 {
  block ^entry:
    %bits = const i64 4294967551
    return u8 %bits
}
function @raw_true() -> i1 {
  block ^entry:
    %bits = const i64 4294967297
    return i1 %bits
}
function @raw_false() -> i1 {
  block ^entry:
    %bits = const i64 4294967296
    return i1 %bits
}
function @forward_signed() -> i8 {
  block ^entry:
    %r = call i8 @raw_signed()
    return i8 %r
}
function @forward_unsigned() -> u8 {
  block ^entry:
    %r = call u8 @raw_unsigned()
    return u8 %r
}
function @forward_true() -> i1 {
  block ^entry:
    %r = call i1 @raw_true()
    return i1 %r
}
function @forward_false() -> i1 {
  block ^entry:
    %r = call i1 @raw_false()
    return i1 %r
}
function @widen_signed() -> i64 {
  block ^entry:
    %r = call i8 @forward_signed()
    return i64 %r
}
function @widen_unsigned() -> i64 {
  block ^entry:
    %r = call u8 @forward_unsigned()
    return i64 %r
}
function @branch_false() -> i64 {
  block ^entry:
    %r = call i1 @forward_false()
    branch %r, ^wrong, ^right
  block ^wrong:
    return i64 1
  block ^right:
    return i64 0
}
function @main() -> i32 [role=entry] {
  block ^entry:
    %s = call i64 @widen_signed()
    %u = call i64 @widen_unsigned()
    %t = call i1 @forward_true()
    %f = call i64 @branch_false()
    %bs = cmp ne i64 %s, -127
    %bu = cmp ne i64 %u, 255
    %bt = cmp ne i1 %t, 1
    %bf = cmp ne i64 %f, 0
    %x = binary or i64 %bs, %bu
    %y = binary or i64 %bt, %bf
    %z = binary or i64 %x, %y
    %exit = convert trunc i32 i64 %z
    return i32 %exit
}
