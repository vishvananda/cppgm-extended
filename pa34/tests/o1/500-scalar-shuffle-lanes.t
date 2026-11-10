# Scalar insertions supply only their own bits. Every selected lane must
# retain those bits or the specified zero, including NaNs and signed zero.
global @inputs : obj<256x16> = {
  i64 11936128515503554560
  i64 -72339069014638593
  i64 11936128517651038208
  i64 -72339069014638593
  i64 11936128517642649600
  i64 -72339069014638593
  i64 11936128519790133248
  i64 -72339069014638593
  i64 11936128517646843989
  i64 -72339069014638593
  i64 11936128517642649685
  i64 -72339069014638593
  i64 11936128515503554561
  i64 -72339069014638593
  i64 11936128518716391424
  i64 -72339069014638593
  i64 0
  i64 -72339069014638593
  i64 9223372036854775808
  i64 -72339069014638593
  i64 9218868437227405312
  i64 -72339069014638593
  i64 18442240474082181120
  i64 -72339069014638593
  i64 9221120237041090645
  i64 -72339069014638593
  i64 9218868437227405397
  i64 -72339069014638593
  i64 1
  i64 -72339069014638593
  i64 13830554455654793216
  i64 -72339069014638593
}
global @output : obj<32x16> = { zero 32 }

function @f32_0(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof32 %poison
    %inserted = vector fromf32 %scalar
    %shuffled = vector pshufd %inserted, 0
    store v128 %shuffled, %out
    return void
}

function @f32_1(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof32 %poison
    %inserted = vector fromf32 %scalar
    %shuffled = vector pshufd %inserted, 1
    store v128 %shuffled, %out
    return void
}

function @f32_85(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof32 %poison
    %inserted = vector fromf32 %scalar
    %shuffled = vector pshufd %inserted, 85
    store v128 %shuffled, %out
    return void
}

function @f32_170(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof32 %poison
    %inserted = vector fromf32 %scalar
    %shuffled = vector pshufd %inserted, 170
    store v128 %shuffled, %out
    return void
}

function @f32_255(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof32 %poison
    %inserted = vector fromf32 %scalar
    %shuffled = vector pshufd %inserted, 255
    store v128 %shuffled, %out
    return void
}

function @f32_27(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof32 %poison
    %inserted = vector fromf32 %scalar
    %shuffled = vector pshufd %inserted, 27
    store v128 %shuffled, %out
    return void
}

function @f64_0(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 0
    store v128 %shuffled, %out
    return void
}

function @f64_1(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 1
    store v128 %shuffled, %out
    return void
}

function @f64_4(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 4
    store v128 %shuffled, %out
    return void
}

function @f64_5(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 5
    store v128 %shuffled, %out
    return void
}

function @f64_16(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 16
    store v128 %shuffled, %out
    return void
}

function @f64_17(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 17
    store v128 %shuffled, %out
    return void
}

function @f64_20(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 20
    store v128 %shuffled, %out
    return void
}

function @f64_21(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 21
    store v128 %shuffled, %out
    return void
}

function @f64_64(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 64
    store v128 %shuffled, %out
    return void
}

function @f64_65(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 65
    store v128 %shuffled, %out
    return void
}

function @f64_68(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 68
    store v128 %shuffled, %out
    return void
}

function @f64_69(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 69
    store v128 %shuffled, %out
    return void
}

function @f64_80(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 80
    store v128 %shuffled, %out
    return void
}

function @f64_81(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 81
    store v128 %shuffled, %out
    return void
}

function @f64_84(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 84
    store v128 %shuffled, %out
    return void
}

function @f64_85(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 85
    store v128 %shuffled, %out
    return void
}

function @f64_170(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 170
    store v128 %shuffled, %out
    return void
}

function @f64_255(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 255
    store v128 %shuffled, %out
    return void
}

function @f64_228(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof64 %poison
    %inserted = vector fromf64 %scalar
    %shuffled = vector pshufd %inserted, 228
    store v128 %shuffled, %out
    return void
}

function @i32_0(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %scalar = load i32 %source
    %inserted = vector fromi32 %scalar
    %shuffled = vector pshufd %inserted, 0
    store v128 %shuffled, %out
    return void
}

function @i64_68(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %scalar = load i64 %source
    %inserted = vector fromi64 %scalar
    %shuffled = vector pshufd %inserted, 68
    store v128 %shuffled, %out
    return void
}

function @live_source(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %poison = load v128 %source
    %scalar = vector tof32 %poison
    %inserted = vector fromf32 %scalar
    %shuffled = vector pshufd %inserted, 0
    store v128 %shuffled, %out
    %tail = index i8 %out, 16
    store f32 %scalar, %tail
    return void
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %base = addr @inputs
    %out = addr @output
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %next]
    %errors = phi i64 [^entry: 0, ^loop: %last]
    %offset = binary mul i64 %i, 16
    %source = index i8 %base, %offset
    %source_hi = index i8 %source, 4
    %lo = load u32 %source
    %hi = load u32 %source_hi
    %out_0 = index i8 %out, 0
    %out_1 = index i8 %out, 4
    %out_2 = index i8 %out, 8
    %out_3 = index i8 %out, 12
    call void @f32_0(%source, %out)
    %got_0_0 = load u32 %out_0
    %bad_0_0 = cmp ne u32 %got_0_0, %lo
    %errors_0_0 = binary or i64 %errors, %bad_0_0
    %got_0_1 = load u32 %out_1
    %bad_0_1 = cmp ne u32 %got_0_1, %lo
    %errors_0_1 = binary or i64 %errors_0_0, %bad_0_1
    %got_0_2 = load u32 %out_2
    %bad_0_2 = cmp ne u32 %got_0_2, %lo
    %errors_0_2 = binary or i64 %errors_0_1, %bad_0_2
    %got_0_3 = load u32 %out_3
    %bad_0_3 = cmp ne u32 %got_0_3, %lo
    %errors_0_3 = binary or i64 %errors_0_2, %bad_0_3
    call void @f32_1(%source, %out)
    %got_1_0 = load u32 %out_0
    %bad_1_0 = cmp ne u32 %got_1_0, 0
    %errors_1_0 = binary or i64 %errors_0_3, %bad_1_0
    %got_1_1 = load u32 %out_1
    %bad_1_1 = cmp ne u32 %got_1_1, %lo
    %errors_1_1 = binary or i64 %errors_1_0, %bad_1_1
    %got_1_2 = load u32 %out_2
    %bad_1_2 = cmp ne u32 %got_1_2, %lo
    %errors_1_2 = binary or i64 %errors_1_1, %bad_1_2
    %got_1_3 = load u32 %out_3
    %bad_1_3 = cmp ne u32 %got_1_3, %lo
    %errors_1_3 = binary or i64 %errors_1_2, %bad_1_3
    call void @f32_85(%source, %out)
    %got_2_0 = load u32 %out_0
    %bad_2_0 = cmp ne u32 %got_2_0, 0
    %errors_2_0 = binary or i64 %errors_1_3, %bad_2_0
    %got_2_1 = load u32 %out_1
    %bad_2_1 = cmp ne u32 %got_2_1, 0
    %errors_2_1 = binary or i64 %errors_2_0, %bad_2_1
    %got_2_2 = load u32 %out_2
    %bad_2_2 = cmp ne u32 %got_2_2, 0
    %errors_2_2 = binary or i64 %errors_2_1, %bad_2_2
    %got_2_3 = load u32 %out_3
    %bad_2_3 = cmp ne u32 %got_2_3, 0
    %errors_2_3 = binary or i64 %errors_2_2, %bad_2_3
    call void @f32_170(%source, %out)
    %got_3_0 = load u32 %out_0
    %bad_3_0 = cmp ne u32 %got_3_0, 0
    %errors_3_0 = binary or i64 %errors_2_3, %bad_3_0
    %got_3_1 = load u32 %out_1
    %bad_3_1 = cmp ne u32 %got_3_1, 0
    %errors_3_1 = binary or i64 %errors_3_0, %bad_3_1
    %got_3_2 = load u32 %out_2
    %bad_3_2 = cmp ne u32 %got_3_2, 0
    %errors_3_2 = binary or i64 %errors_3_1, %bad_3_2
    %got_3_3 = load u32 %out_3
    %bad_3_3 = cmp ne u32 %got_3_3, 0
    %errors_3_3 = binary or i64 %errors_3_2, %bad_3_3
    call void @f32_255(%source, %out)
    %got_4_0 = load u32 %out_0
    %bad_4_0 = cmp ne u32 %got_4_0, 0
    %errors_4_0 = binary or i64 %errors_3_3, %bad_4_0
    %got_4_1 = load u32 %out_1
    %bad_4_1 = cmp ne u32 %got_4_1, 0
    %errors_4_1 = binary or i64 %errors_4_0, %bad_4_1
    %got_4_2 = load u32 %out_2
    %bad_4_2 = cmp ne u32 %got_4_2, 0
    %errors_4_2 = binary or i64 %errors_4_1, %bad_4_2
    %got_4_3 = load u32 %out_3
    %bad_4_3 = cmp ne u32 %got_4_3, 0
    %errors_4_3 = binary or i64 %errors_4_2, %bad_4_3
    call void @f32_27(%source, %out)
    %got_5_0 = load u32 %out_0
    %bad_5_0 = cmp ne u32 %got_5_0, 0
    %errors_5_0 = binary or i64 %errors_4_3, %bad_5_0
    %got_5_1 = load u32 %out_1
    %bad_5_1 = cmp ne u32 %got_5_1, 0
    %errors_5_1 = binary or i64 %errors_5_0, %bad_5_1
    %got_5_2 = load u32 %out_2
    %bad_5_2 = cmp ne u32 %got_5_2, 0
    %errors_5_2 = binary or i64 %errors_5_1, %bad_5_2
    %got_5_3 = load u32 %out_3
    %bad_5_3 = cmp ne u32 %got_5_3, %lo
    %errors_5_3 = binary or i64 %errors_5_2, %bad_5_3
    call void @f64_0(%source, %out)
    %got_6_0 = load u32 %out_0
    %bad_6_0 = cmp ne u32 %got_6_0, %lo
    %errors_6_0 = binary or i64 %errors_5_3, %bad_6_0
    %got_6_1 = load u32 %out_1
    %bad_6_1 = cmp ne u32 %got_6_1, %lo
    %errors_6_1 = binary or i64 %errors_6_0, %bad_6_1
    %got_6_2 = load u32 %out_2
    %bad_6_2 = cmp ne u32 %got_6_2, %lo
    %errors_6_2 = binary or i64 %errors_6_1, %bad_6_2
    %got_6_3 = load u32 %out_3
    %bad_6_3 = cmp ne u32 %got_6_3, %lo
    %errors_6_3 = binary or i64 %errors_6_2, %bad_6_3
    call void @f64_1(%source, %out)
    %got_7_0 = load u32 %out_0
    %bad_7_0 = cmp ne u32 %got_7_0, %hi
    %errors_7_0 = binary or i64 %errors_6_3, %bad_7_0
    %got_7_1 = load u32 %out_1
    %bad_7_1 = cmp ne u32 %got_7_1, %lo
    %errors_7_1 = binary or i64 %errors_7_0, %bad_7_1
    %got_7_2 = load u32 %out_2
    %bad_7_2 = cmp ne u32 %got_7_2, %lo
    %errors_7_2 = binary or i64 %errors_7_1, %bad_7_2
    %got_7_3 = load u32 %out_3
    %bad_7_3 = cmp ne u32 %got_7_3, %lo
    %errors_7_3 = binary or i64 %errors_7_2, %bad_7_3
    call void @f64_4(%source, %out)
    %got_8_0 = load u32 %out_0
    %bad_8_0 = cmp ne u32 %got_8_0, %lo
    %errors_8_0 = binary or i64 %errors_7_3, %bad_8_0
    %got_8_1 = load u32 %out_1
    %bad_8_1 = cmp ne u32 %got_8_1, %hi
    %errors_8_1 = binary or i64 %errors_8_0, %bad_8_1
    %got_8_2 = load u32 %out_2
    %bad_8_2 = cmp ne u32 %got_8_2, %lo
    %errors_8_2 = binary or i64 %errors_8_1, %bad_8_2
    %got_8_3 = load u32 %out_3
    %bad_8_3 = cmp ne u32 %got_8_3, %lo
    %errors_8_3 = binary or i64 %errors_8_2, %bad_8_3
    call void @f64_5(%source, %out)
    %got_9_0 = load u32 %out_0
    %bad_9_0 = cmp ne u32 %got_9_0, %hi
    %errors_9_0 = binary or i64 %errors_8_3, %bad_9_0
    %got_9_1 = load u32 %out_1
    %bad_9_1 = cmp ne u32 %got_9_1, %hi
    %errors_9_1 = binary or i64 %errors_9_0, %bad_9_1
    %got_9_2 = load u32 %out_2
    %bad_9_2 = cmp ne u32 %got_9_2, %lo
    %errors_9_2 = binary or i64 %errors_9_1, %bad_9_2
    %got_9_3 = load u32 %out_3
    %bad_9_3 = cmp ne u32 %got_9_3, %lo
    %errors_9_3 = binary or i64 %errors_9_2, %bad_9_3
    call void @f64_16(%source, %out)
    %got_10_0 = load u32 %out_0
    %bad_10_0 = cmp ne u32 %got_10_0, %lo
    %errors_10_0 = binary or i64 %errors_9_3, %bad_10_0
    %got_10_1 = load u32 %out_1
    %bad_10_1 = cmp ne u32 %got_10_1, %lo
    %errors_10_1 = binary or i64 %errors_10_0, %bad_10_1
    %got_10_2 = load u32 %out_2
    %bad_10_2 = cmp ne u32 %got_10_2, %hi
    %errors_10_2 = binary or i64 %errors_10_1, %bad_10_2
    %got_10_3 = load u32 %out_3
    %bad_10_3 = cmp ne u32 %got_10_3, %lo
    %errors_10_3 = binary or i64 %errors_10_2, %bad_10_3
    call void @f64_17(%source, %out)
    %got_11_0 = load u32 %out_0
    %bad_11_0 = cmp ne u32 %got_11_0, %hi
    %errors_11_0 = binary or i64 %errors_10_3, %bad_11_0
    %got_11_1 = load u32 %out_1
    %bad_11_1 = cmp ne u32 %got_11_1, %lo
    %errors_11_1 = binary or i64 %errors_11_0, %bad_11_1
    %got_11_2 = load u32 %out_2
    %bad_11_2 = cmp ne u32 %got_11_2, %hi
    %errors_11_2 = binary or i64 %errors_11_1, %bad_11_2
    %got_11_3 = load u32 %out_3
    %bad_11_3 = cmp ne u32 %got_11_3, %lo
    %errors_11_3 = binary or i64 %errors_11_2, %bad_11_3
    call void @f64_20(%source, %out)
    %got_12_0 = load u32 %out_0
    %bad_12_0 = cmp ne u32 %got_12_0, %lo
    %errors_12_0 = binary or i64 %errors_11_3, %bad_12_0
    %got_12_1 = load u32 %out_1
    %bad_12_1 = cmp ne u32 %got_12_1, %hi
    %errors_12_1 = binary or i64 %errors_12_0, %bad_12_1
    %got_12_2 = load u32 %out_2
    %bad_12_2 = cmp ne u32 %got_12_2, %hi
    %errors_12_2 = binary or i64 %errors_12_1, %bad_12_2
    %got_12_3 = load u32 %out_3
    %bad_12_3 = cmp ne u32 %got_12_3, %lo
    %errors_12_3 = binary or i64 %errors_12_2, %bad_12_3
    call void @f64_21(%source, %out)
    %got_13_0 = load u32 %out_0
    %bad_13_0 = cmp ne u32 %got_13_0, %hi
    %errors_13_0 = binary or i64 %errors_12_3, %bad_13_0
    %got_13_1 = load u32 %out_1
    %bad_13_1 = cmp ne u32 %got_13_1, %hi
    %errors_13_1 = binary or i64 %errors_13_0, %bad_13_1
    %got_13_2 = load u32 %out_2
    %bad_13_2 = cmp ne u32 %got_13_2, %hi
    %errors_13_2 = binary or i64 %errors_13_1, %bad_13_2
    %got_13_3 = load u32 %out_3
    %bad_13_3 = cmp ne u32 %got_13_3, %lo
    %errors_13_3 = binary or i64 %errors_13_2, %bad_13_3
    call void @f64_64(%source, %out)
    %got_14_0 = load u32 %out_0
    %bad_14_0 = cmp ne u32 %got_14_0, %lo
    %errors_14_0 = binary or i64 %errors_13_3, %bad_14_0
    %got_14_1 = load u32 %out_1
    %bad_14_1 = cmp ne u32 %got_14_1, %lo
    %errors_14_1 = binary or i64 %errors_14_0, %bad_14_1
    %got_14_2 = load u32 %out_2
    %bad_14_2 = cmp ne u32 %got_14_2, %lo
    %errors_14_2 = binary or i64 %errors_14_1, %bad_14_2
    %got_14_3 = load u32 %out_3
    %bad_14_3 = cmp ne u32 %got_14_3, %hi
    %errors_14_3 = binary or i64 %errors_14_2, %bad_14_3
    call void @f64_65(%source, %out)
    %got_15_0 = load u32 %out_0
    %bad_15_0 = cmp ne u32 %got_15_0, %hi
    %errors_15_0 = binary or i64 %errors_14_3, %bad_15_0
    %got_15_1 = load u32 %out_1
    %bad_15_1 = cmp ne u32 %got_15_1, %lo
    %errors_15_1 = binary or i64 %errors_15_0, %bad_15_1
    %got_15_2 = load u32 %out_2
    %bad_15_2 = cmp ne u32 %got_15_2, %lo
    %errors_15_2 = binary or i64 %errors_15_1, %bad_15_2
    %got_15_3 = load u32 %out_3
    %bad_15_3 = cmp ne u32 %got_15_3, %hi
    %errors_15_3 = binary or i64 %errors_15_2, %bad_15_3
    call void @f64_68(%source, %out)
    %got_16_0 = load u32 %out_0
    %bad_16_0 = cmp ne u32 %got_16_0, %lo
    %errors_16_0 = binary or i64 %errors_15_3, %bad_16_0
    %got_16_1 = load u32 %out_1
    %bad_16_1 = cmp ne u32 %got_16_1, %hi
    %errors_16_1 = binary or i64 %errors_16_0, %bad_16_1
    %got_16_2 = load u32 %out_2
    %bad_16_2 = cmp ne u32 %got_16_2, %lo
    %errors_16_2 = binary or i64 %errors_16_1, %bad_16_2
    %got_16_3 = load u32 %out_3
    %bad_16_3 = cmp ne u32 %got_16_3, %hi
    %errors_16_3 = binary or i64 %errors_16_2, %bad_16_3
    call void @f64_69(%source, %out)
    %got_17_0 = load u32 %out_0
    %bad_17_0 = cmp ne u32 %got_17_0, %hi
    %errors_17_0 = binary or i64 %errors_16_3, %bad_17_0
    %got_17_1 = load u32 %out_1
    %bad_17_1 = cmp ne u32 %got_17_1, %hi
    %errors_17_1 = binary or i64 %errors_17_0, %bad_17_1
    %got_17_2 = load u32 %out_2
    %bad_17_2 = cmp ne u32 %got_17_2, %lo
    %errors_17_2 = binary or i64 %errors_17_1, %bad_17_2
    %got_17_3 = load u32 %out_3
    %bad_17_3 = cmp ne u32 %got_17_3, %hi
    %errors_17_3 = binary or i64 %errors_17_2, %bad_17_3
    call void @f64_80(%source, %out)
    %got_18_0 = load u32 %out_0
    %bad_18_0 = cmp ne u32 %got_18_0, %lo
    %errors_18_0 = binary or i64 %errors_17_3, %bad_18_0
    %got_18_1 = load u32 %out_1
    %bad_18_1 = cmp ne u32 %got_18_1, %lo
    %errors_18_1 = binary or i64 %errors_18_0, %bad_18_1
    %got_18_2 = load u32 %out_2
    %bad_18_2 = cmp ne u32 %got_18_2, %hi
    %errors_18_2 = binary or i64 %errors_18_1, %bad_18_2
    %got_18_3 = load u32 %out_3
    %bad_18_3 = cmp ne u32 %got_18_3, %hi
    %errors_18_3 = binary or i64 %errors_18_2, %bad_18_3
    call void @f64_81(%source, %out)
    %got_19_0 = load u32 %out_0
    %bad_19_0 = cmp ne u32 %got_19_0, %hi
    %errors_19_0 = binary or i64 %errors_18_3, %bad_19_0
    %got_19_1 = load u32 %out_1
    %bad_19_1 = cmp ne u32 %got_19_1, %lo
    %errors_19_1 = binary or i64 %errors_19_0, %bad_19_1
    %got_19_2 = load u32 %out_2
    %bad_19_2 = cmp ne u32 %got_19_2, %hi
    %errors_19_2 = binary or i64 %errors_19_1, %bad_19_2
    %got_19_3 = load u32 %out_3
    %bad_19_3 = cmp ne u32 %got_19_3, %hi
    %errors_19_3 = binary or i64 %errors_19_2, %bad_19_3
    call void @f64_84(%source, %out)
    %got_20_0 = load u32 %out_0
    %bad_20_0 = cmp ne u32 %got_20_0, %lo
    %errors_20_0 = binary or i64 %errors_19_3, %bad_20_0
    %got_20_1 = load u32 %out_1
    %bad_20_1 = cmp ne u32 %got_20_1, %hi
    %errors_20_1 = binary or i64 %errors_20_0, %bad_20_1
    %got_20_2 = load u32 %out_2
    %bad_20_2 = cmp ne u32 %got_20_2, %hi
    %errors_20_2 = binary or i64 %errors_20_1, %bad_20_2
    %got_20_3 = load u32 %out_3
    %bad_20_3 = cmp ne u32 %got_20_3, %hi
    %errors_20_3 = binary or i64 %errors_20_2, %bad_20_3
    call void @f64_85(%source, %out)
    %got_21_0 = load u32 %out_0
    %bad_21_0 = cmp ne u32 %got_21_0, %hi
    %errors_21_0 = binary or i64 %errors_20_3, %bad_21_0
    %got_21_1 = load u32 %out_1
    %bad_21_1 = cmp ne u32 %got_21_1, %hi
    %errors_21_1 = binary or i64 %errors_21_0, %bad_21_1
    %got_21_2 = load u32 %out_2
    %bad_21_2 = cmp ne u32 %got_21_2, %hi
    %errors_21_2 = binary or i64 %errors_21_1, %bad_21_2
    %got_21_3 = load u32 %out_3
    %bad_21_3 = cmp ne u32 %got_21_3, %hi
    %errors_21_3 = binary or i64 %errors_21_2, %bad_21_3
    call void @f64_170(%source, %out)
    %got_22_0 = load u32 %out_0
    %bad_22_0 = cmp ne u32 %got_22_0, 0
    %errors_22_0 = binary or i64 %errors_21_3, %bad_22_0
    %got_22_1 = load u32 %out_1
    %bad_22_1 = cmp ne u32 %got_22_1, 0
    %errors_22_1 = binary or i64 %errors_22_0, %bad_22_1
    %got_22_2 = load u32 %out_2
    %bad_22_2 = cmp ne u32 %got_22_2, 0
    %errors_22_2 = binary or i64 %errors_22_1, %bad_22_2
    %got_22_3 = load u32 %out_3
    %bad_22_3 = cmp ne u32 %got_22_3, 0
    %errors_22_3 = binary or i64 %errors_22_2, %bad_22_3
    call void @f64_255(%source, %out)
    %got_23_0 = load u32 %out_0
    %bad_23_0 = cmp ne u32 %got_23_0, 0
    %errors_23_0 = binary or i64 %errors_22_3, %bad_23_0
    %got_23_1 = load u32 %out_1
    %bad_23_1 = cmp ne u32 %got_23_1, 0
    %errors_23_1 = binary or i64 %errors_23_0, %bad_23_1
    %got_23_2 = load u32 %out_2
    %bad_23_2 = cmp ne u32 %got_23_2, 0
    %errors_23_2 = binary or i64 %errors_23_1, %bad_23_2
    %got_23_3 = load u32 %out_3
    %bad_23_3 = cmp ne u32 %got_23_3, 0
    %errors_23_3 = binary or i64 %errors_23_2, %bad_23_3
    call void @f64_228(%source, %out)
    %got_24_0 = load u32 %out_0
    %bad_24_0 = cmp ne u32 %got_24_0, %lo
    %errors_24_0 = binary or i64 %errors_23_3, %bad_24_0
    %got_24_1 = load u32 %out_1
    %bad_24_1 = cmp ne u32 %got_24_1, %hi
    %errors_24_1 = binary or i64 %errors_24_0, %bad_24_1
    %got_24_2 = load u32 %out_2
    %bad_24_2 = cmp ne u32 %got_24_2, 0
    %errors_24_2 = binary or i64 %errors_24_1, %bad_24_2
    %got_24_3 = load u32 %out_3
    %bad_24_3 = cmp ne u32 %got_24_3, 0
    %errors_24_3 = binary or i64 %errors_24_2, %bad_24_3
    call void @i32_0(%source, %out)
    %got_25_0 = load u32 %out_0
    %bad_25_0 = cmp ne u32 %got_25_0, %lo
    %errors_25_0 = binary or i64 %errors_24_3, %bad_25_0
    %got_25_1 = load u32 %out_1
    %bad_25_1 = cmp ne u32 %got_25_1, %lo
    %errors_25_1 = binary or i64 %errors_25_0, %bad_25_1
    %got_25_2 = load u32 %out_2
    %bad_25_2 = cmp ne u32 %got_25_2, %lo
    %errors_25_2 = binary or i64 %errors_25_1, %bad_25_2
    %got_25_3 = load u32 %out_3
    %bad_25_3 = cmp ne u32 %got_25_3, %lo
    %errors_25_3 = binary or i64 %errors_25_2, %bad_25_3
    call void @i64_68(%source, %out)
    %got_26_0 = load u32 %out_0
    %bad_26_0 = cmp ne u32 %got_26_0, %lo
    %errors_26_0 = binary or i64 %errors_25_3, %bad_26_0
    %got_26_1 = load u32 %out_1
    %bad_26_1 = cmp ne u32 %got_26_1, %hi
    %errors_26_1 = binary or i64 %errors_26_0, %bad_26_1
    %got_26_2 = load u32 %out_2
    %bad_26_2 = cmp ne u32 %got_26_2, %lo
    %errors_26_2 = binary or i64 %errors_26_1, %bad_26_2
    %got_26_3 = load u32 %out_3
    %bad_26_3 = cmp ne u32 %got_26_3, %hi
    %errors_26_3 = binary or i64 %errors_26_2, %bad_26_3
    call void @live_source(%source, %out)
    %live_place_0 = index i8 %out, 0
    %live_got_0 = load u32 %live_place_0
    %live_bad_0 = cmp ne u32 %live_got_0, %lo
    %live_errors_0 = binary or i64 %errors_26_3, %live_bad_0
    %live_place_1 = index i8 %out, 4
    %live_got_1 = load u32 %live_place_1
    %live_bad_1 = cmp ne u32 %live_got_1, %lo
    %live_errors_1 = binary or i64 %live_errors_0, %live_bad_1
    %live_place_2 = index i8 %out, 8
    %live_got_2 = load u32 %live_place_2
    %live_bad_2 = cmp ne u32 %live_got_2, %lo
    %live_errors_2 = binary or i64 %live_errors_1, %live_bad_2
    %live_place_3 = index i8 %out, 12
    %live_got_3 = load u32 %live_place_3
    %live_bad_3 = cmp ne u32 %live_got_3, %lo
    %live_errors_3 = binary or i64 %live_errors_2, %live_bad_3
    %live_place_4 = index i8 %out, 16
    %live_got_4 = load u32 %live_place_4
    %live_bad_4 = cmp ne u32 %live_got_4, %lo
    %live_errors_4 = binary or i64 %live_errors_3, %live_bad_4
    %last = copy i64 %live_errors_4
    %next = binary add i64 %i, 1
    %again = cmp lt i64 %next, 16
    branch %again, ^loop, ^exit
  block ^exit:
    return i64 %last
}
