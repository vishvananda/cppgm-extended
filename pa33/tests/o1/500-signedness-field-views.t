# Signed and unsigned views of one width need not pack adjacent fields.
# True byte overlap, Boolean conversion, copies and volatile accesses retain
# their representation and observation rules.
function @byte_counter(%seed : i8, %rounds : i32) -> i64 [no_inline=yes] {
  slot $state : obj<8x4>
  block ^entry:
    %base = addr $state
    %byte = index i8 [projection=field] %base, 4
    store i32 0, %base
    store i8 %seed, %byte
    jump ^loop
  block ^loop:
    %sum = phi i64 [^entry: 0, ^next: %total]
    %i = load i32 %base
    %more = cmp lt i32 %i, %rounds
    branch %more, ^body, ^done
  block ^body:
    %b = load u8 %byte
    %wide = convert zext i64 u8 %b
    %iw = convert sext i64 i32 %i
    %part = binary add i64 %wide, %iw
    %total = binary add i64 %sum, %part
    %incremented = binary add u8 %b, 1
    store i8 %incremented, %byte
    %inext = binary add i32 %i, 1
    store i32 %inext, %base
    jump ^next
  block ^next:
    jump ^loop
  block ^done:
    return i64 %sum
}

function @width_views(%a : i8, %b : u16, %c : i32, %d : i64) -> i64 [no_inline=yes] {
  slot $value : obj<16x8>
  block ^entry:
    %base = addr $value
    %at2 = index i8 [projection=field] %base, 2
    %at4 = index i8 [projection=field] %base, 4
    %at8 = index i8 [projection=field] %base, 8
    store i8 %a, %base
    store u16 %b, %at2
    store i32 %c, %at4
    store i64 %d, %at8
    %av = load u8 %base
    %bv = load i16 %at2
    %cv = load u32 %at4
    %dv = load i64 %at8
    %aw = convert zext i64 u8 %av
    %bw = convert sext i64 i16 %bv
    %cw = convert zext i64 u32 %cv
    %ab = binary add i64 %aw, %bw
    %cd = binary add i64 %cw, %dv
    %sum = binary add i64 %ab, %cd
    return i64 %sum
}

function @copy_views(%x : i32, %y : u16, %choose : i1) -> i64 [no_inline=yes] {
  slot $source : obj<8x4>
  slot $target : obj<8x4>
  block ^entry:
    %s = addr $source
    %s4 = index i8 [projection=field] %s, 4
    %t = addr $target
    %t4 = index i8 [projection=field] %t, 4
    store i32 %x, %s
    store u16 %y, %s4
    store u32 7, %t
    store i16 -2, %t4
    branch %choose, ^copy, ^read
  block ^copy:
    copyobj 8x4 %s, %t
    jump ^read
  block ^read:
    %low = load u32 %t
    %high = load i16 %t4
    %lw = convert zext i64 u32 %low
    %hw = convert sext i64 i16 %high
    %sum = binary add i64 %lw, %hw
    return i64 %sum
}

function @overlap(%bits : i64, %byte : i8) -> u32 [no_inline=yes] {
  slot $value : obj<8x8>
  block ^entry:
    %base = addr $value
    %at2 = index i8 [projection=field] %base, 2
    store i64 %bits, %base
    store i8 %byte, %at2
    %result = load u32 %base
    return u32 %result
}

function @boolean_conversion(%input : u8) -> u8 [no_inline=yes] {
  slot $value : obj<8x4>
  block ^entry:
    %base = addr $value
    %bit = convert trunc i1 u8 %input
    store i1 %bit, %base
    %byte = load u8 %base
    return u8 %byte
}

function @volatile_view(%input : i8) -> u8 [no_inline=yes] {
  slot $value : obj<8x4>
  block ^entry:
    %base = addr $value
    store volatile i8 %input, %base
    %byte = load volatile u8 %base
    return u8 %byte
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @byte_counter(-1, 16)
    %b = call i64 @byte_counter(127, 1)
    %c = call i64 @byte_counter(-128, 2)
    %d = call i64 @byte_counter(-1, 0)
    %e = call i64 @width_views(-1, 65534, -3, -4)
    %f = call i64 @copy_views(-3, 65534, 1)
    %g = call i64 @copy_views(-3, 65534, 0)
    %h = call u32 @overlap(1234605616436508552, -1)
    %i = call u8 @boolean_conversion(2)
    %j = call u8 @boolean_conversion(3)
    %k = call u8 @volatile_view(-1)
    %ok_a = cmp eq i64 %a, 480
    %ok_b = cmp eq i64 %b, 127
    %ok_c = cmp eq i64 %c, 258
    %ok_d = cmp eq i64 %d, 0
    %ok_e = cmp eq i64 %e, 4294967542
    %ok_f = cmp eq i64 %f, 4294967291
    %ok_g = cmp eq i64 %g, 5
    %ok_h = cmp eq u32 %h, 1442805640
    %ok_i = cmp eq u8 %i, 0
    %ok_j = cmp eq u8 %j, 1
    %ok_k = cmp eq u8 %k, 255
    %ab = binary and i1 %ok_a, %ok_b
    %cd = binary and i1 %ok_c, %ok_d
    %ef = binary and i1 %ok_e, %ok_f
    %gh = binary and i1 %ok_g, %ok_h
    %ij = binary and i1 %ok_i, %ok_j
    %abcd = binary and i1 %ab, %cd
    %efgh = binary and i1 %ef, %gh
    %ijk = binary and i1 %ij, %ok_k
    %abcdefgh = binary and i1 %abcd, %efgh
    %all = binary and i1 %abcdefgh, %ijk
    branch %all, ^pass, ^fail
  block ^pass:
    return i32 0
  block ^fail:
    return i32 1
}
