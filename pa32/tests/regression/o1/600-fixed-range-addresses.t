# Two addresses at known offsets from one base subtract to the difference of
# the offsets and compare as the offsets do: @span's end less its begin is 64
# bytes, 8 elements, and its tests are literals. A memmove of a literal size
# becomes a copyobj when its ranges cannot overlap: from one slot to another,
# or between two halves of one slot. @shift's ranges overlap (offsets 0 and
# 4 of 8 bytes): its call stays.
declare function @__builtin_memmove(%arg0 : ptr, %arg1 : ptr, %arg2 : i64) -> ptr [unwind=no, binding=strong, object=cppgm_builtin_memmove]

function @span() -> i64 [no_inline=yes] {
  slot $a : obj<64x8>

  block ^entry:
    %b = addr $a
    %e = index i8 %b, 64
    %bytes = binary sub ptr %e, %b
    %count = binary shr exact i64 %bytes, 3
    %some = cmp ne ptr %b, %e
    %back = cmp ult ptr %e, %b
    %one = convert zext i64 i1 %some
    %zero = convert zext i64 i1 %back
    %t = binary add i64 %count, %one
    %r = binary add i64 %t, %zero
    return i64 %r
}

function @copied(%x : i64, %y : i64) -> i64 [no_inline=yes] {
  slot $from : obj<16x8>
  slot $to : obj<16x8>
  slot $both : obj<16x8>

  block ^entry:
    %f = addr $from
    store i64 %x, %f
    %f1 = index i8 %f, 8
    store i64 %y, %f1
    %t = addr $to
    %m = call ptr @__builtin_memmove(%t, %f, 16)
    %b = addr $both
    store i64 %x, %b
    %b1 = index i8 %b, 8
    store i64 %y, %b1
    %h = call ptr @__builtin_memmove(%b1, %b, 8)
    %t1 = index i8 %t, 8
    %u = load i64 %t1
    %v = load i64 %b1
    %r = binary sub i64 %u, %v
    return i64 %r
}

function @shift(%x : i64, %y : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>

  block ^entry:
    %p = addr $s
    store i64 %x, %p
    %p1 = index i8 %p, 8
    store i64 %y, %p1
    %q = index i8 %p, 4
    %m = call ptr @__builtin_memmove(%q, %p, 8)
    %w = load i64 %p1
    return i64 %w
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %n = call i64 @span()
    %d = call i64 @copied(5, 7)
    %w = call i64 @shift(4294967296, 0)
    %bad_n = cmp ne i64 %n, 9
    %bad_d = cmp ne i64 %d, 2
    %bad_w = cmp ne i64 %w, 1
    %a = convert zext i64 i1 %bad_n
    %b = convert zext i64 i1 %bad_d
    %c = convert zext i64 i1 %bad_w
    %ab = binary or i64 %a, %b
    %abc = binary or i64 %ab, %c
    %status = convert trunc i32 i64 %abc
    return i32 %status
}
