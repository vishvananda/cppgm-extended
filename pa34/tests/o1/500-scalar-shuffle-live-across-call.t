# Values survive an ABI call, spilled scalar reloads and a later scalar use.
global @inputs : obj<80x16> = {
  i32 1056964608
  i32 1057039173
  i32 1057113738
  i32 1057188303
  i32 1057262868
  i32 1057337433
  i32 1057411998
  i32 1057486563
  i32 1057561128
  i32 1057635693
  i32 1057710258
  i32 1057784823
  i32 1057859388
  i32 1057933953
  i32 1058008518
  i32 1058083083
  i32 1058157648
  i32 1058232213
  i32 1058306778
  i32 1058381343
}
global @output : obj<400x16> = { zero 400 }
global @seen : f64 = 0

function @clobber(%x0 : f64, %x1 : f64, %x2 : f64, %x3 : f64, %x4 : f64, %x5 : f64, %x6 : f64, %x7 : f64, %x8 : f64, %x9 : f64) -> void {
  block ^entry:
    store f64 %x7, @seen
    return void
}

function @pressure(%src : ptr, %out : ptr) -> void {
  block ^entry:
    %p0 = index i8 %src, 0
    %x0 = load volatile f32 %p0
    %p1 = index i8 %src, 4
    %x1 = load volatile f32 %p1
    %p2 = index i8 %src, 8
    %x2 = load volatile f32 %p2
    %p3 = index i8 %src, 12
    %x3 = load volatile f32 %p3
    %p4 = index i8 %src, 16
    %x4 = load volatile f32 %p4
    %p5 = index i8 %src, 20
    %x5 = load volatile f32 %p5
    %p6 = index i8 %src, 24
    %x6 = load volatile f32 %p6
    %p7 = index i8 %src, 28
    %x7 = load volatile f32 %p7
    %p8 = index i8 %src, 32
    %x8 = load volatile f32 %p8
    %p9 = index i8 %src, 36
    %x9 = load volatile f32 %p9
    %p10 = index i8 %src, 40
    %x10 = load volatile f32 %p10
    %p11 = index i8 %src, 44
    %x11 = load volatile f32 %p11
    %p12 = index i8 %src, 48
    %x12 = load volatile f32 %p12
    %p13 = index i8 %src, 52
    %x13 = load volatile f32 %p13
    %p14 = index i8 %src, 56
    %x14 = load volatile f32 %p14
    %p15 = index i8 %src, 60
    %x15 = load volatile f32 %p15
    %p16 = index i8 %src, 64
    %x16 = load volatile f32 %p16
    %p17 = index i8 %src, 68
    %x17 = load volatile f32 %p17
    %p18 = index i8 %src, 72
    %x18 = load volatile f32 %p18
    %p19 = index i8 %src, 76
    %x19 = load volatile f32 %p19
    call void @clobber(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0)
    %v0 = vector fromf32 %x0
    %b0 = vector pshufd %v0, 0
    %o0 = index i8 %out, 0
    store v128 %b0, %o0
    %v1 = vector fromf32 %x1
    %b1 = vector pshufd %v1, 0
    %o1 = index i8 %out, 16
    store v128 %b1, %o1
    %v2 = vector fromf32 %x2
    %b2 = vector pshufd %v2, 0
    %o2 = index i8 %out, 32
    store v128 %b2, %o2
    %v3 = vector fromf32 %x3
    %b3 = vector pshufd %v3, 0
    %o3 = index i8 %out, 48
    store v128 %b3, %o3
    %v4 = vector fromf32 %x4
    %b4 = vector pshufd %v4, 0
    %o4 = index i8 %out, 64
    store v128 %b4, %o4
    %v5 = vector fromf32 %x5
    %b5 = vector pshufd %v5, 0
    %o5 = index i8 %out, 80
    store v128 %b5, %o5
    %v6 = vector fromf32 %x6
    %b6 = vector pshufd %v6, 0
    %o6 = index i8 %out, 96
    store v128 %b6, %o6
    %v7 = vector fromf32 %x7
    %b7 = vector pshufd %v7, 0
    %o7 = index i8 %out, 112
    store v128 %b7, %o7
    %v8 = vector fromf32 %x8
    %b8 = vector pshufd %v8, 0
    %o8 = index i8 %out, 128
    store v128 %b8, %o8
    %v9 = vector fromf32 %x9
    %b9 = vector pshufd %v9, 0
    %o9 = index i8 %out, 144
    store v128 %b9, %o9
    %v10 = vector fromf32 %x10
    %b10 = vector pshufd %v10, 0
    %o10 = index i8 %out, 160
    store v128 %b10, %o10
    %v11 = vector fromf32 %x11
    %b11 = vector pshufd %v11, 0
    %o11 = index i8 %out, 176
    store v128 %b11, %o11
    %v12 = vector fromf32 %x12
    %b12 = vector pshufd %v12, 0
    %o12 = index i8 %out, 192
    store v128 %b12, %o12
    %v13 = vector fromf32 %x13
    %b13 = vector pshufd %v13, 0
    %o13 = index i8 %out, 208
    store v128 %b13, %o13
    %v14 = vector fromf32 %x14
    %b14 = vector pshufd %v14, 0
    %o14 = index i8 %out, 224
    store v128 %b14, %o14
    %v15 = vector fromf32 %x15
    %b15 = vector pshufd %v15, 0
    %o15 = index i8 %out, 240
    store v128 %b15, %o15
    %v16 = vector fromf32 %x16
    %b16 = vector pshufd %v16, 0
    %o16 = index i8 %out, 256
    store v128 %b16, %o16
    %v17 = vector fromf32 %x17
    %b17 = vector pshufd %v17, 0
    %o17 = index i8 %out, 272
    store v128 %b17, %o17
    %v18 = vector fromf32 %x18
    %b18 = vector pshufd %v18, 0
    %o18 = index i8 %out, 288
    store v128 %b18, %o18
    %v19 = vector fromf32 %x19
    %b19 = vector pshufd %v19, 0
    %o19 = index i8 %out, 304
    store v128 %b19, %o19
    %t0 = index i8 %out, 320
    store f32 %x0, %t0
    %t1 = index i8 %out, 324
    store f32 %x1, %t1
    %t2 = index i8 %out, 328
    store f32 %x2, %t2
    %t3 = index i8 %out, 332
    store f32 %x3, %t3
    %t4 = index i8 %out, 336
    store f32 %x4, %t4
    %t5 = index i8 %out, 340
    store f32 %x5, %t5
    %t6 = index i8 %out, 344
    store f32 %x6, %t6
    %t7 = index i8 %out, 348
    store f32 %x7, %t7
    %t8 = index i8 %out, 352
    store f32 %x8, %t8
    %t9 = index i8 %out, 356
    store f32 %x9, %t9
    %t10 = index i8 %out, 360
    store f32 %x10, %t10
    %t11 = index i8 %out, 364
    store f32 %x11, %t11
    %t12 = index i8 %out, 368
    store f32 %x12, %t12
    %t13 = index i8 %out, 372
    store f32 %x13, %t13
    %t14 = index i8 %out, 376
    store f32 %x14, %t14
    %t15 = index i8 %out, 380
    store f32 %x15, %t15
    %t16 = index i8 %out, 384
    store f32 %x16, %t16
    %t17 = index i8 %out, 388
    store f32 %x17, %t17
    %t18 = index i8 %out, 392
    store f32 %x18, %t18
    %t19 = index i8 %out, 396
    store f32 %x19, %t19
    return void
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %src = addr @inputs
    %out = addr @output
    call void @pressure(%src, %out)
    %p0_0 = index i8 %out, 0
    %got0_0 = load u32 %p0_0
    %bad0_0 = cmp ne u32 %got0_0, 1056964608
    %error0_0 = binary or i64 0, %bad0_0
    %p0_1 = index i8 %out, 4
    %got0_1 = load u32 %p0_1
    %bad0_1 = cmp ne u32 %got0_1, 1056964608
    %error0_1 = binary or i64 %error0_0, %bad0_1
    %p0_2 = index i8 %out, 8
    %got0_2 = load u32 %p0_2
    %bad0_2 = cmp ne u32 %got0_2, 1056964608
    %error0_2 = binary or i64 %error0_1, %bad0_2
    %p0_3 = index i8 %out, 12
    %got0_3 = load u32 %p0_3
    %bad0_3 = cmp ne u32 %got0_3, 1056964608
    %error0_3 = binary or i64 %error0_2, %bad0_3
    %p0_4 = index i8 %out, 320
    %got0_4 = load u32 %p0_4
    %bad0_4 = cmp ne u32 %got0_4, 1056964608
    %error0_4 = binary or i64 %error0_3, %bad0_4
    %p1_0 = index i8 %out, 16
    %got1_0 = load u32 %p1_0
    %bad1_0 = cmp ne u32 %got1_0, 1057039173
    %error1_0 = binary or i64 %error0_4, %bad1_0
    %p1_1 = index i8 %out, 20
    %got1_1 = load u32 %p1_1
    %bad1_1 = cmp ne u32 %got1_1, 1057039173
    %error1_1 = binary or i64 %error1_0, %bad1_1
    %p1_2 = index i8 %out, 24
    %got1_2 = load u32 %p1_2
    %bad1_2 = cmp ne u32 %got1_2, 1057039173
    %error1_2 = binary or i64 %error1_1, %bad1_2
    %p1_3 = index i8 %out, 28
    %got1_3 = load u32 %p1_3
    %bad1_3 = cmp ne u32 %got1_3, 1057039173
    %error1_3 = binary or i64 %error1_2, %bad1_3
    %p1_4 = index i8 %out, 324
    %got1_4 = load u32 %p1_4
    %bad1_4 = cmp ne u32 %got1_4, 1057039173
    %error1_4 = binary or i64 %error1_3, %bad1_4
    %p2_0 = index i8 %out, 32
    %got2_0 = load u32 %p2_0
    %bad2_0 = cmp ne u32 %got2_0, 1057113738
    %error2_0 = binary or i64 %error1_4, %bad2_0
    %p2_1 = index i8 %out, 36
    %got2_1 = load u32 %p2_1
    %bad2_1 = cmp ne u32 %got2_1, 1057113738
    %error2_1 = binary or i64 %error2_0, %bad2_1
    %p2_2 = index i8 %out, 40
    %got2_2 = load u32 %p2_2
    %bad2_2 = cmp ne u32 %got2_2, 1057113738
    %error2_2 = binary or i64 %error2_1, %bad2_2
    %p2_3 = index i8 %out, 44
    %got2_3 = load u32 %p2_3
    %bad2_3 = cmp ne u32 %got2_3, 1057113738
    %error2_3 = binary or i64 %error2_2, %bad2_3
    %p2_4 = index i8 %out, 328
    %got2_4 = load u32 %p2_4
    %bad2_4 = cmp ne u32 %got2_4, 1057113738
    %error2_4 = binary or i64 %error2_3, %bad2_4
    %p3_0 = index i8 %out, 48
    %got3_0 = load u32 %p3_0
    %bad3_0 = cmp ne u32 %got3_0, 1057188303
    %error3_0 = binary or i64 %error2_4, %bad3_0
    %p3_1 = index i8 %out, 52
    %got3_1 = load u32 %p3_1
    %bad3_1 = cmp ne u32 %got3_1, 1057188303
    %error3_1 = binary or i64 %error3_0, %bad3_1
    %p3_2 = index i8 %out, 56
    %got3_2 = load u32 %p3_2
    %bad3_2 = cmp ne u32 %got3_2, 1057188303
    %error3_2 = binary or i64 %error3_1, %bad3_2
    %p3_3 = index i8 %out, 60
    %got3_3 = load u32 %p3_3
    %bad3_3 = cmp ne u32 %got3_3, 1057188303
    %error3_3 = binary or i64 %error3_2, %bad3_3
    %p3_4 = index i8 %out, 332
    %got3_4 = load u32 %p3_4
    %bad3_4 = cmp ne u32 %got3_4, 1057188303
    %error3_4 = binary or i64 %error3_3, %bad3_4
    %p4_0 = index i8 %out, 64
    %got4_0 = load u32 %p4_0
    %bad4_0 = cmp ne u32 %got4_0, 1057262868
    %error4_0 = binary or i64 %error3_4, %bad4_0
    %p4_1 = index i8 %out, 68
    %got4_1 = load u32 %p4_1
    %bad4_1 = cmp ne u32 %got4_1, 1057262868
    %error4_1 = binary or i64 %error4_0, %bad4_1
    %p4_2 = index i8 %out, 72
    %got4_2 = load u32 %p4_2
    %bad4_2 = cmp ne u32 %got4_2, 1057262868
    %error4_2 = binary or i64 %error4_1, %bad4_2
    %p4_3 = index i8 %out, 76
    %got4_3 = load u32 %p4_3
    %bad4_3 = cmp ne u32 %got4_3, 1057262868
    %error4_3 = binary or i64 %error4_2, %bad4_3
    %p4_4 = index i8 %out, 336
    %got4_4 = load u32 %p4_4
    %bad4_4 = cmp ne u32 %got4_4, 1057262868
    %error4_4 = binary or i64 %error4_3, %bad4_4
    %p5_0 = index i8 %out, 80
    %got5_0 = load u32 %p5_0
    %bad5_0 = cmp ne u32 %got5_0, 1057337433
    %error5_0 = binary or i64 %error4_4, %bad5_0
    %p5_1 = index i8 %out, 84
    %got5_1 = load u32 %p5_1
    %bad5_1 = cmp ne u32 %got5_1, 1057337433
    %error5_1 = binary or i64 %error5_0, %bad5_1
    %p5_2 = index i8 %out, 88
    %got5_2 = load u32 %p5_2
    %bad5_2 = cmp ne u32 %got5_2, 1057337433
    %error5_2 = binary or i64 %error5_1, %bad5_2
    %p5_3 = index i8 %out, 92
    %got5_3 = load u32 %p5_3
    %bad5_3 = cmp ne u32 %got5_3, 1057337433
    %error5_3 = binary or i64 %error5_2, %bad5_3
    %p5_4 = index i8 %out, 340
    %got5_4 = load u32 %p5_4
    %bad5_4 = cmp ne u32 %got5_4, 1057337433
    %error5_4 = binary or i64 %error5_3, %bad5_4
    %p6_0 = index i8 %out, 96
    %got6_0 = load u32 %p6_0
    %bad6_0 = cmp ne u32 %got6_0, 1057411998
    %error6_0 = binary or i64 %error5_4, %bad6_0
    %p6_1 = index i8 %out, 100
    %got6_1 = load u32 %p6_1
    %bad6_1 = cmp ne u32 %got6_1, 1057411998
    %error6_1 = binary or i64 %error6_0, %bad6_1
    %p6_2 = index i8 %out, 104
    %got6_2 = load u32 %p6_2
    %bad6_2 = cmp ne u32 %got6_2, 1057411998
    %error6_2 = binary or i64 %error6_1, %bad6_2
    %p6_3 = index i8 %out, 108
    %got6_3 = load u32 %p6_3
    %bad6_3 = cmp ne u32 %got6_3, 1057411998
    %error6_3 = binary or i64 %error6_2, %bad6_3
    %p6_4 = index i8 %out, 344
    %got6_4 = load u32 %p6_4
    %bad6_4 = cmp ne u32 %got6_4, 1057411998
    %error6_4 = binary or i64 %error6_3, %bad6_4
    %p7_0 = index i8 %out, 112
    %got7_0 = load u32 %p7_0
    %bad7_0 = cmp ne u32 %got7_0, 1057486563
    %error7_0 = binary or i64 %error6_4, %bad7_0
    %p7_1 = index i8 %out, 116
    %got7_1 = load u32 %p7_1
    %bad7_1 = cmp ne u32 %got7_1, 1057486563
    %error7_1 = binary or i64 %error7_0, %bad7_1
    %p7_2 = index i8 %out, 120
    %got7_2 = load u32 %p7_2
    %bad7_2 = cmp ne u32 %got7_2, 1057486563
    %error7_2 = binary or i64 %error7_1, %bad7_2
    %p7_3 = index i8 %out, 124
    %got7_3 = load u32 %p7_3
    %bad7_3 = cmp ne u32 %got7_3, 1057486563
    %error7_3 = binary or i64 %error7_2, %bad7_3
    %p7_4 = index i8 %out, 348
    %got7_4 = load u32 %p7_4
    %bad7_4 = cmp ne u32 %got7_4, 1057486563
    %error7_4 = binary or i64 %error7_3, %bad7_4
    %p8_0 = index i8 %out, 128
    %got8_0 = load u32 %p8_0
    %bad8_0 = cmp ne u32 %got8_0, 1057561128
    %error8_0 = binary or i64 %error7_4, %bad8_0
    %p8_1 = index i8 %out, 132
    %got8_1 = load u32 %p8_1
    %bad8_1 = cmp ne u32 %got8_1, 1057561128
    %error8_1 = binary or i64 %error8_0, %bad8_1
    %p8_2 = index i8 %out, 136
    %got8_2 = load u32 %p8_2
    %bad8_2 = cmp ne u32 %got8_2, 1057561128
    %error8_2 = binary or i64 %error8_1, %bad8_2
    %p8_3 = index i8 %out, 140
    %got8_3 = load u32 %p8_3
    %bad8_3 = cmp ne u32 %got8_3, 1057561128
    %error8_3 = binary or i64 %error8_2, %bad8_3
    %p8_4 = index i8 %out, 352
    %got8_4 = load u32 %p8_4
    %bad8_4 = cmp ne u32 %got8_4, 1057561128
    %error8_4 = binary or i64 %error8_3, %bad8_4
    %p9_0 = index i8 %out, 144
    %got9_0 = load u32 %p9_0
    %bad9_0 = cmp ne u32 %got9_0, 1057635693
    %error9_0 = binary or i64 %error8_4, %bad9_0
    %p9_1 = index i8 %out, 148
    %got9_1 = load u32 %p9_1
    %bad9_1 = cmp ne u32 %got9_1, 1057635693
    %error9_1 = binary or i64 %error9_0, %bad9_1
    %p9_2 = index i8 %out, 152
    %got9_2 = load u32 %p9_2
    %bad9_2 = cmp ne u32 %got9_2, 1057635693
    %error9_2 = binary or i64 %error9_1, %bad9_2
    %p9_3 = index i8 %out, 156
    %got9_3 = load u32 %p9_3
    %bad9_3 = cmp ne u32 %got9_3, 1057635693
    %error9_3 = binary or i64 %error9_2, %bad9_3
    %p9_4 = index i8 %out, 356
    %got9_4 = load u32 %p9_4
    %bad9_4 = cmp ne u32 %got9_4, 1057635693
    %error9_4 = binary or i64 %error9_3, %bad9_4
    %p10_0 = index i8 %out, 160
    %got10_0 = load u32 %p10_0
    %bad10_0 = cmp ne u32 %got10_0, 1057710258
    %error10_0 = binary or i64 %error9_4, %bad10_0
    %p10_1 = index i8 %out, 164
    %got10_1 = load u32 %p10_1
    %bad10_1 = cmp ne u32 %got10_1, 1057710258
    %error10_1 = binary or i64 %error10_0, %bad10_1
    %p10_2 = index i8 %out, 168
    %got10_2 = load u32 %p10_2
    %bad10_2 = cmp ne u32 %got10_2, 1057710258
    %error10_2 = binary or i64 %error10_1, %bad10_2
    %p10_3 = index i8 %out, 172
    %got10_3 = load u32 %p10_3
    %bad10_3 = cmp ne u32 %got10_3, 1057710258
    %error10_3 = binary or i64 %error10_2, %bad10_3
    %p10_4 = index i8 %out, 360
    %got10_4 = load u32 %p10_4
    %bad10_4 = cmp ne u32 %got10_4, 1057710258
    %error10_4 = binary or i64 %error10_3, %bad10_4
    %p11_0 = index i8 %out, 176
    %got11_0 = load u32 %p11_0
    %bad11_0 = cmp ne u32 %got11_0, 1057784823
    %error11_0 = binary or i64 %error10_4, %bad11_0
    %p11_1 = index i8 %out, 180
    %got11_1 = load u32 %p11_1
    %bad11_1 = cmp ne u32 %got11_1, 1057784823
    %error11_1 = binary or i64 %error11_0, %bad11_1
    %p11_2 = index i8 %out, 184
    %got11_2 = load u32 %p11_2
    %bad11_2 = cmp ne u32 %got11_2, 1057784823
    %error11_2 = binary or i64 %error11_1, %bad11_2
    %p11_3 = index i8 %out, 188
    %got11_3 = load u32 %p11_3
    %bad11_3 = cmp ne u32 %got11_3, 1057784823
    %error11_3 = binary or i64 %error11_2, %bad11_3
    %p11_4 = index i8 %out, 364
    %got11_4 = load u32 %p11_4
    %bad11_4 = cmp ne u32 %got11_4, 1057784823
    %error11_4 = binary or i64 %error11_3, %bad11_4
    %p12_0 = index i8 %out, 192
    %got12_0 = load u32 %p12_0
    %bad12_0 = cmp ne u32 %got12_0, 1057859388
    %error12_0 = binary or i64 %error11_4, %bad12_0
    %p12_1 = index i8 %out, 196
    %got12_1 = load u32 %p12_1
    %bad12_1 = cmp ne u32 %got12_1, 1057859388
    %error12_1 = binary or i64 %error12_0, %bad12_1
    %p12_2 = index i8 %out, 200
    %got12_2 = load u32 %p12_2
    %bad12_2 = cmp ne u32 %got12_2, 1057859388
    %error12_2 = binary or i64 %error12_1, %bad12_2
    %p12_3 = index i8 %out, 204
    %got12_3 = load u32 %p12_3
    %bad12_3 = cmp ne u32 %got12_3, 1057859388
    %error12_3 = binary or i64 %error12_2, %bad12_3
    %p12_4 = index i8 %out, 368
    %got12_4 = load u32 %p12_4
    %bad12_4 = cmp ne u32 %got12_4, 1057859388
    %error12_4 = binary or i64 %error12_3, %bad12_4
    %p13_0 = index i8 %out, 208
    %got13_0 = load u32 %p13_0
    %bad13_0 = cmp ne u32 %got13_0, 1057933953
    %error13_0 = binary or i64 %error12_4, %bad13_0
    %p13_1 = index i8 %out, 212
    %got13_1 = load u32 %p13_1
    %bad13_1 = cmp ne u32 %got13_1, 1057933953
    %error13_1 = binary or i64 %error13_0, %bad13_1
    %p13_2 = index i8 %out, 216
    %got13_2 = load u32 %p13_2
    %bad13_2 = cmp ne u32 %got13_2, 1057933953
    %error13_2 = binary or i64 %error13_1, %bad13_2
    %p13_3 = index i8 %out, 220
    %got13_3 = load u32 %p13_3
    %bad13_3 = cmp ne u32 %got13_3, 1057933953
    %error13_3 = binary or i64 %error13_2, %bad13_3
    %p13_4 = index i8 %out, 372
    %got13_4 = load u32 %p13_4
    %bad13_4 = cmp ne u32 %got13_4, 1057933953
    %error13_4 = binary or i64 %error13_3, %bad13_4
    %p14_0 = index i8 %out, 224
    %got14_0 = load u32 %p14_0
    %bad14_0 = cmp ne u32 %got14_0, 1058008518
    %error14_0 = binary or i64 %error13_4, %bad14_0
    %p14_1 = index i8 %out, 228
    %got14_1 = load u32 %p14_1
    %bad14_1 = cmp ne u32 %got14_1, 1058008518
    %error14_1 = binary or i64 %error14_0, %bad14_1
    %p14_2 = index i8 %out, 232
    %got14_2 = load u32 %p14_2
    %bad14_2 = cmp ne u32 %got14_2, 1058008518
    %error14_2 = binary or i64 %error14_1, %bad14_2
    %p14_3 = index i8 %out, 236
    %got14_3 = load u32 %p14_3
    %bad14_3 = cmp ne u32 %got14_3, 1058008518
    %error14_3 = binary or i64 %error14_2, %bad14_3
    %p14_4 = index i8 %out, 376
    %got14_4 = load u32 %p14_4
    %bad14_4 = cmp ne u32 %got14_4, 1058008518
    %error14_4 = binary or i64 %error14_3, %bad14_4
    %p15_0 = index i8 %out, 240
    %got15_0 = load u32 %p15_0
    %bad15_0 = cmp ne u32 %got15_0, 1058083083
    %error15_0 = binary or i64 %error14_4, %bad15_0
    %p15_1 = index i8 %out, 244
    %got15_1 = load u32 %p15_1
    %bad15_1 = cmp ne u32 %got15_1, 1058083083
    %error15_1 = binary or i64 %error15_0, %bad15_1
    %p15_2 = index i8 %out, 248
    %got15_2 = load u32 %p15_2
    %bad15_2 = cmp ne u32 %got15_2, 1058083083
    %error15_2 = binary or i64 %error15_1, %bad15_2
    %p15_3 = index i8 %out, 252
    %got15_3 = load u32 %p15_3
    %bad15_3 = cmp ne u32 %got15_3, 1058083083
    %error15_3 = binary or i64 %error15_2, %bad15_3
    %p15_4 = index i8 %out, 380
    %got15_4 = load u32 %p15_4
    %bad15_4 = cmp ne u32 %got15_4, 1058083083
    %error15_4 = binary or i64 %error15_3, %bad15_4
    %p16_0 = index i8 %out, 256
    %got16_0 = load u32 %p16_0
    %bad16_0 = cmp ne u32 %got16_0, 1058157648
    %error16_0 = binary or i64 %error15_4, %bad16_0
    %p16_1 = index i8 %out, 260
    %got16_1 = load u32 %p16_1
    %bad16_1 = cmp ne u32 %got16_1, 1058157648
    %error16_1 = binary or i64 %error16_0, %bad16_1
    %p16_2 = index i8 %out, 264
    %got16_2 = load u32 %p16_2
    %bad16_2 = cmp ne u32 %got16_2, 1058157648
    %error16_2 = binary or i64 %error16_1, %bad16_2
    %p16_3 = index i8 %out, 268
    %got16_3 = load u32 %p16_3
    %bad16_3 = cmp ne u32 %got16_3, 1058157648
    %error16_3 = binary or i64 %error16_2, %bad16_3
    %p16_4 = index i8 %out, 384
    %got16_4 = load u32 %p16_4
    %bad16_4 = cmp ne u32 %got16_4, 1058157648
    %error16_4 = binary or i64 %error16_3, %bad16_4
    %p17_0 = index i8 %out, 272
    %got17_0 = load u32 %p17_0
    %bad17_0 = cmp ne u32 %got17_0, 1058232213
    %error17_0 = binary or i64 %error16_4, %bad17_0
    %p17_1 = index i8 %out, 276
    %got17_1 = load u32 %p17_1
    %bad17_1 = cmp ne u32 %got17_1, 1058232213
    %error17_1 = binary or i64 %error17_0, %bad17_1
    %p17_2 = index i8 %out, 280
    %got17_2 = load u32 %p17_2
    %bad17_2 = cmp ne u32 %got17_2, 1058232213
    %error17_2 = binary or i64 %error17_1, %bad17_2
    %p17_3 = index i8 %out, 284
    %got17_3 = load u32 %p17_3
    %bad17_3 = cmp ne u32 %got17_3, 1058232213
    %error17_3 = binary or i64 %error17_2, %bad17_3
    %p17_4 = index i8 %out, 388
    %got17_4 = load u32 %p17_4
    %bad17_4 = cmp ne u32 %got17_4, 1058232213
    %error17_4 = binary or i64 %error17_3, %bad17_4
    %p18_0 = index i8 %out, 288
    %got18_0 = load u32 %p18_0
    %bad18_0 = cmp ne u32 %got18_0, 1058306778
    %error18_0 = binary or i64 %error17_4, %bad18_0
    %p18_1 = index i8 %out, 292
    %got18_1 = load u32 %p18_1
    %bad18_1 = cmp ne u32 %got18_1, 1058306778
    %error18_1 = binary or i64 %error18_0, %bad18_1
    %p18_2 = index i8 %out, 296
    %got18_2 = load u32 %p18_2
    %bad18_2 = cmp ne u32 %got18_2, 1058306778
    %error18_2 = binary or i64 %error18_1, %bad18_2
    %p18_3 = index i8 %out, 300
    %got18_3 = load u32 %p18_3
    %bad18_3 = cmp ne u32 %got18_3, 1058306778
    %error18_3 = binary or i64 %error18_2, %bad18_3
    %p18_4 = index i8 %out, 392
    %got18_4 = load u32 %p18_4
    %bad18_4 = cmp ne u32 %got18_4, 1058306778
    %error18_4 = binary or i64 %error18_3, %bad18_4
    %p19_0 = index i8 %out, 304
    %got19_0 = load u32 %p19_0
    %bad19_0 = cmp ne u32 %got19_0, 1058381343
    %error19_0 = binary or i64 %error18_4, %bad19_0
    %p19_1 = index i8 %out, 308
    %got19_1 = load u32 %p19_1
    %bad19_1 = cmp ne u32 %got19_1, 1058381343
    %error19_1 = binary or i64 %error19_0, %bad19_1
    %p19_2 = index i8 %out, 312
    %got19_2 = load u32 %p19_2
    %bad19_2 = cmp ne u32 %got19_2, 1058381343
    %error19_2 = binary or i64 %error19_1, %bad19_2
    %p19_3 = index i8 %out, 316
    %got19_3 = load u32 %p19_3
    %bad19_3 = cmp ne u32 %got19_3, 1058381343
    %error19_3 = binary or i64 %error19_2, %bad19_3
    %p19_4 = index i8 %out, 396
    %got19_4 = load u32 %p19_4
    %bad19_4 = cmp ne u32 %got19_4, 1058381343
    %error19_4 = binary or i64 %error19_3, %bad19_4
    return i64 %error19_4
}
