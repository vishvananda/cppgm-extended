# The lane read of a value just placed in that lane is the value. A wider or
# narrower read, and a shufpd that takes the second lane, keep the vector.
function @scalar_round_trip(%x : f64, %y : f32, %z : i32, %w : i64) -> i64 {
  block ^entry:
    %a = vector fromf64 %x
    %ax = vector tof64 %a
    %b = vector fromf32 %y
    %by = vector tof32 %b
    %c = vector fromi32 %z
    %cz = vector toi32 %c
    %d = vector fromi64 %w
    %dw = vector toi64 %d
    %wide = convert fptosi i64 f64 %ax
    %narrow = convert fptosi i64 f32 %by
    %word = convert sext i64 i32 %cz
    %one = binary add i64 %wide, %narrow
    %two = binary add i64 %word, %dw
    %all = binary add i64 %one, %two
    return i64 %all
}

function @low_of_unpack(%x : f64, %y : f64) -> f64 {
  block ^entry:
    %a = vector fromf64 %x
    %b = vector fromf64 %y
    %pair = vector unpcklpd %a, %b
    %low = vector tof64 %pair
    return f64 %low
}

function @low_of_float_unpack(%x : f32, %y : f32) -> f32 {
  block ^entry:
    %a = vector fromf32 %x
    %b = vector fromf32 %y
    %pair = vector unpcklps %a, %b
    %low = vector tof32 %pair
    return f32 %low
}

function @wide_read_of_float_unpack(%x : f32, %y : f32) -> f64 {
  block ^entry:
    %a = vector fromf32 %x
    %b = vector fromf32 %y
    %pair = vector unpcklps %a, %b
    %low = vector tof64 %pair
    return f64 %low
}

function @low_of_shufpd(%x : f64, %v : v128) -> f64 {
  block ^entry:
    %a = vector fromf64 %x
    %pair = vector shufpd %a, %v, 2
    %low = vector tof64 %pair
    return f64 %low
}

function @second_lane_of_shufpd(%x : f64, %v : v128) -> f64 {
  block ^entry:
    %a = vector fromf64 %x
    %pair = vector shufpd %a, %v, 1
    %low = vector tof64 %pair
    return f64 %low
}

function @unpack_of_unpack(%a : v128, %c : v128, %d : v128) -> v128 {
  block ^entry:
    %inner = vector unpcklpd %c, %d
    %outer = vector unpcklpd %a, %inner
    return v128 %outer
}

function @unpack_of_high(%a : v128, %z : v128) -> v128 {
  block ^entry:
    %high = vector unpckhpd %z, %z
    %outer = vector unpcklpd %a, %high
    return v128 %outer
}

# unpcklps reads two lanes of each operand, so a value rebuilt from its first
# lane is not the value.
function @float_unpack_keeps_second_lane(%a : v128, %z : v128) -> v128 {
  block ^entry:
    %low = vector tof32 %z
    %rebuilt = vector fromf32 %low
    %outer = vector unpcklps %a, %rebuilt
    return v128 %outer
}

function @float_unpack_reads_low_double(%a : v128, %z : v128) -> v128 {
  block ^entry:
    %low = vector tof64 %z
    %rebuilt = vector fromf64 %low
    %outer = vector unpcklps %a, %rebuilt
    return v128 %outer
}

function @word_lane_of_interleave(%x : i32, %v : v128) -> i32 {
  block ^entry:
    %a = vector fromi32 %x
    %pair = vector punpckldq %a, %v
    %low = vector toi32 %pair
    return i32 %low
}

function @quad_read_of_word_interleave(%x : i32, %v : v128) -> i64 {
  block ^entry:
    %a = vector fromi32 %x
    %pair = vector punpckldq %a, %v
    %low = vector toi64 %pair
    return i64 %low
}
