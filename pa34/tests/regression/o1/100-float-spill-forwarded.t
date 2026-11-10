# A floating value spilled for its next read is read from its register: the
# load would wait for the store.
global @input : obj<160x8> = { zero 160 }
global @output : obj<8x8> = { zero 8 }

function @many() -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %p0 = index f64 @input, 0
    %v0 = load typed f64 %p0
    %p1 = index f64 @input, 1
    %v1 = load typed f64 %p1
    %p2 = index f64 @input, 2
    %v2 = load typed f64 %p2
    %p3 = index f64 @input, 3
    %v3 = load typed f64 %p3
    %p4 = index f64 @input, 4
    %v4 = load typed f64 %p4
    %p5 = index f64 @input, 5
    %v5 = load typed f64 %p5
    %p6 = index f64 @input, 6
    %v6 = load typed f64 %p6
    %p7 = index f64 @input, 7
    %v7 = load typed f64 %p7
    %p8 = index f64 @input, 8
    %v8 = load typed f64 %p8
    %p9 = index f64 @input, 9
    %v9 = load typed f64 %p9
    %p10 = index f64 @input, 10
    %v10 = load typed f64 %p10
    %p11 = index f64 @input, 11
    %v11 = load typed f64 %p11
    %p12 = index f64 @input, 12
    %v12 = load typed f64 %p12
    %p13 = index f64 @input, 13
    %v13 = load typed f64 %p13
    %p14 = index f64 @input, 14
    %v14 = load typed f64 %p14
    %p15 = index f64 @input, 15
    %v15 = load typed f64 %p15
    %p16 = index f64 @input, 16
    %v16 = load typed f64 %p16
    %p17 = index f64 @input, 17
    %v17 = load typed f64 %p17
    %m1 = binary mul f64 %v1, %v1
    %m2 = binary mul f64 %v2, %v2
    %m3 = binary mul f64 %v3, %v3
    %m4 = binary mul f64 %v4, %v4
    %m5 = binary mul f64 %v5, %v5
    %m6 = binary mul f64 %v6, %v6
    %m7 = binary mul f64 %v7, %v7
    %m8 = binary mul f64 %v8, %v8
    %m9 = binary mul f64 %v9, %v9
    %m10 = binary mul f64 %v10, %v10
    %m11 = binary mul f64 %v11, %v11
    %m12 = binary mul f64 %v12, %v12
    %m13 = binary mul f64 %v13, %v13
    %m14 = binary mul f64 %v14, %v14
    %m15 = binary mul f64 %v15, %v15
    %m16 = binary mul f64 %v16, %v16
    %m17 = binary mul f64 %v17, %v17
    %s1 = binary add f64 %v0, %m1
    %s2 = binary add f64 %s1, %m2
    %s3 = binary add f64 %s2, %m3
    %s4 = binary add f64 %s3, %m4
    %s5 = binary add f64 %s4, %m5
    %s6 = binary add f64 %s5, %m6
    %s7 = binary add f64 %s6, %m7
    %s8 = binary add f64 %s7, %m8
    %s9 = binary add f64 %s8, %m9
    %s10 = binary add f64 %s9, %m10
    %s11 = binary add f64 %s10, %m11
    %s12 = binary add f64 %s11, %m12
    %s13 = binary add f64 %s12, %m13
    %s14 = binary add f64 %s13, %m14
    %s15 = binary add f64 %s14, %m15
    %s16 = binary add f64 %s15, %m16
    %s17 = binary add f64 %s16, %m17
    %d0 = binary add f64 %s17, %v0
    %d1 = binary add f64 %d0, %v1
    %d2 = binary add f64 %d1, %v2
    %d3 = binary add f64 %d2, %v3
    %d4 = binary add f64 %d3, %v4
    %d5 = binary add f64 %d4, %v5
    %d6 = binary add f64 %d5, %v6
    %d7 = binary add f64 %d6, %v7
    %d8 = binary add f64 %d7, %v8
    %d9 = binary add f64 %d8, %v9
    %d10 = binary add f64 %d9, %v10
    %d11 = binary add f64 %d10, %v11
    %d12 = binary add f64 %d11, %v12
    %d13 = binary add f64 %d12, %v13
    %d14 = binary add f64 %d13, %v14
    %d15 = binary add f64 %d14, %v15
    %d16 = binary add f64 %d15, %v16
    %d17 = binary add f64 %d16, %v17
    store typed f64 %d17, @output
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %p = index f64 @input, 1
    store typed f64 2, %p
    call void @many()
    %r = load typed f64 @output
    %w = convert fptosi i32 f64 %r
    %z = binary sub i32 %w, 6
    return i32 %z
}
