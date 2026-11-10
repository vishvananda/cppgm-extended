function @low_signed(%p : obj<8x8>) -> i32 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 [projection=field] %home, 0
    %v = load i32 %field
    return i32 %v
}

function @high_unsigned(%p : obj<8x8>) -> u32 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 [projection=field] %home, 4
    %v = load u32 %field
    return u32 %v
}

function @high_signed(%p : obj<8x8>) -> i32 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 [projection=field] %home, 4
    %v = load i32 %field
    return i32 %v
}

function @last_signed(%p : obj<8x8>) -> i8 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 [projection=field] %home, 7
    %v = load i8 %field
    return i8 %v
}

function @inner_unsigned(%p : obj<8x8>) -> u8 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 [projection=field] %home, 3
    %v = load u8 %field
    return u8 %v
}

function @unaligned_signed(%p : obj<8x8>) -> i16 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 [projection=field] %home, 1
    %v = load i16 %field
    return i16 %v
}

function @unaligned_unsigned(%p : obj<8x8>) -> u16 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 [projection=field] %home, 3
    %v = load u16 %field
    return u16 %v
}

function @last_half(%p : obj<8x8>) -> u16 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 [projection=field] %home, 6
    %v = load u16 %field
    return u16 %v
}

function @second_high(%p : obj<16x8>) -> i32 [no_inline=yes] {
  slot $home : obj<16x8>
  block ^entry:
    %home = addr $home
    copyobj 16x8 %p, %home
    %field = index i8 [projection=field] %home, 12
    %v = load i32 %field
    return i32 %v
}

function @cross_carriers(%p : obj<16x8>) -> u32 [no_inline=yes] {
  slot $home : obj<16x8>
  block ^entry:
    %home = addr $home
    copyobj 16x8 %p, %home
    %field = index i8 [projection=field] %home, 6
    %v = load u32 %field
    return u32 %v
}

function @after_mutation(%p : obj<8x8>) -> u32 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %changed = index i8 %home, 5
    store u8 31, %changed
    %field = index i8 [projection=field] %home, 4
    %v = load u32 %field
    return u32 %v
}

function @after_escape(%p : obj<8x8>) -> u32 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    call void @stomp(%home)
    %field = index i8 [projection=field] %home, 4
    %v = load u32 %field
    return u32 %v
}

function @after_call(%p : obj<8x8>) -> u32 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    call void @clobber(91, 82, 73, 64, 55, 46)
    %field = index i8 [projection=field] %home, 4
    %v = load u32 %field
    return u32 %v
}

function @observed_field(%p : obj<8x8>) -> i32 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 [projection=field] %home, 4
    %v = load volatile i32 %field
    return i32 %v
}

function @mixed_carriers(%p : obj<16x8> [classes=integer_sse]) -> i32 [no_inline=yes] {
  slot $home : obj<16x8>
  block ^entry:
    %home = addr $home
    copyobj 16x8 %p, %home
    %field = index i8 [projection=field] %home, 4
    %v = load i32 %field
    return i32 %v
}

function @sse_carrier(%p : obj<8x8> [classes=sse]) -> i32 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 [projection=field] %home, 4
    %v = load i32 %field
    return i32 %v
}

function @retained_frame(%p : obj<8x8>) -> i32 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 [projection=field] %home, 4
    %v = load i32 %field
    call void @stomp(%home)
    return i32 %v
}

function @stomp(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    %field = index i8 %p, 4
    store u32 4275878552, %field
    return void
}
function @clobber(%a : i64, %b : i64, %c : i64, %d : i64, %e : i64, %f : i64) -> void [no_inline=yes] {
  block ^entry:
    return void
}
function @both_fields(%p : obj<8x8>) -> i64 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 %home, 4
    %high = load u32 %field
    %low = load u32 %home
    %h = convert zext i64 u32 %high
    %l = convert zext i64 u32 %low
    %v = binary add i64 %h, %l
    return i64 %v
}
function @branch_field(%p : obj<8x8>, %choose : i32) -> i32 [no_inline=yes] {
  slot $home : obj<8x8>
  block ^entry:
    %home = addr $home
    copyobj 8x8 %p, %home
    %field = index i8 %home, 4
    %value = load i32 %field
    %yes = cmp ne i32 %choose, 0
    branch %yes, ^take, ^other
  block ^take:
    return i32 %value
  block ^other:
    %low = load i32 %home
    return i32 %low
}
function @main() -> i32 [role=entry] {
  slot $small : obj<8x8>
  slot $large : obj<16x8>
  block ^entry:
    %small = addr $small
    %large = addr $large

    store i64 -1017017724017666168, %small
    %v0 = call i32 @low_signed($small)
    %b0 = cmp ne i32 %v0, -1247373432
    %e0 = convert zext i32 i1 %b0
    store i64 -1017017724017666168, %small
    %v1 = call u32 @high_unsigned($small)
    %b1 = cmp ne u32 %v1, 4058174404
    %e1 = convert zext i32 i1 %b1
    store i64 -1017017724017666168, %small
    %v2 = call i32 @high_signed($small)
    %b2 = cmp ne i32 %v2, -236792892
    %e2 = convert zext i32 i1 %b2
    store i64 -1017017724017666168, %small
    %v3 = call i8 @last_signed($small)
    %b3 = cmp ne i8 %v3, -15
    %e3 = convert zext i32 i1 %b3
    store i64 -1017017724017666168, %small
    %v4 = call u8 @inner_unsigned($small)
    %b4 = cmp ne u8 %v4, 181
    %e4 = convert zext i32 i1 %b4
    store i64 -1017017724017666168, %small
    %v5 = call i16 @unaligned_signed($small)
    %b5 = cmp ne i16 %v5, -22889
    %e5 = convert zext i32 i1 %b5
    store i64 -1017017724017666168, %small
    %v6 = call u16 @unaligned_unsigned($small)
    %b6 = cmp ne u16 %v6, 50357
    %e6 = convert zext i32 i1 %b6
    store i64 -1017017724017666168, %small
    %v7 = call u16 @last_half($small)
    %b7 = cmp ne u16 %v7, 61922
    %e7 = convert zext i32 i1 %b7
    store i64 -1017017724017666168, %large
    %end8 = index i8 %large, 8
    store i64 2970769764550348688, %end8
    %v8 = call i32 @second_high($large)
    %b8 = cmp ne i32 %v8, 691686236
    %e8 = convert zext i32 i1 %b8
    store i64 -1017017724017666168, %large
    %end9 = index i8 %large, 8
    store i64 2970769764550348688, %end9
    %v9 = call u32 @cross_carriers($large)
    %b9 = cmp ne u32 %v9, 2408640994
    %e9 = convert zext i32 i1 %b9
    store i64 -1017017724017666168, %small
    %v10 = call u32 @after_mutation($small)
    %b10 = cmp ne u32 %v10, 4058128324
    %e10 = convert zext i32 i1 %b10
    store i64 -1017017724017666168, %small
    %v11 = call u32 @after_escape($small)
    %b11 = cmp ne u32 %v11, 4275878552
    %e11 = convert zext i32 i1 %b11
    store i64 -1017017724017666168, %small
    %v12 = call u32 @after_call($small)
    %b12 = cmp ne u32 %v12, 4058174404
    %e12 = convert zext i32 i1 %b12
    store i64 -1017017724017666168, %small
    %v13 = call i32 @observed_field($small)
    %b13 = cmp ne i32 %v13, -236792892
    %e13 = convert zext i32 i1 %b13
    store i64 -1017017724017666168, %large
    %end14 = index i8 %large, 8
    store i64 2970769764550348688, %end14
    %v14 = call i32 @mixed_carriers($large) as (%p : obj<16x8> [classes=integer_sse]) -> i32
    %b14 = cmp ne i32 %v14, -236792892
    %e14 = convert zext i32 i1 %b14
    store i64 -1017017724017666168, %small
    %v15 = call i32 @sse_carrier($small) as (%p : obj<8x8> [classes=sse]) -> i32
    %b15 = cmp ne i32 %v15, -236792892
    %e15 = convert zext i32 i1 %b15
    store i64 -1017017724017666168, %small
    %v16 = call i32 @retained_frame($small)
    %b16 = cmp ne i32 %v16, -236792892
    %e16 = convert zext i32 i1 %b16
    store i64 -1017017724017666168, %small
    %both = call i64 @both_fields($small)
    %bad_both = cmp ne i64 %both, 7105768268
    %e17 = convert zext i32 i1 %bad_both
    %v18 = call i32 @branch_field($small, 0)
    %bad18 = cmp ne i32 %v18, -1247373432
    %e18 = convert zext i32 i1 %bad18
    %v19 = call i32 @branch_field($small, 1)
    %bad19 = cmp ne i32 %v19, -236792892
    %e19 = convert zext i32 i1 %bad19
    %sum1 = binary add i32 %e0, %e1
    %sum2 = binary add i32 %sum1, %e2
    %sum3 = binary add i32 %sum2, %e3
    %sum4 = binary add i32 %sum3, %e4
    %sum5 = binary add i32 %sum4, %e5
    %sum6 = binary add i32 %sum5, %e6
    %sum7 = binary add i32 %sum6, %e7
    %sum8 = binary add i32 %sum7, %e8
    %sum9 = binary add i32 %sum8, %e9
    %sum10 = binary add i32 %sum9, %e10
    %sum11 = binary add i32 %sum10, %e11
    %sum12 = binary add i32 %sum11, %e12
    %sum13 = binary add i32 %sum12, %e13
    %sum14 = binary add i32 %sum13, %e14
    %sum15 = binary add i32 %sum14, %e15
    %sum16 = binary add i32 %sum15, %e16
    %sum17 = binary add i32 %sum16, %e17
    %sum18 = binary add i32 %sum17, %e18
    %sum19 = binary add i32 %sum18, %e19
    return i32 %sum19
}
