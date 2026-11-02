# Sixteen values live to the end of the block and fourteen registers hold them.
# Each short product takes the register of the value that lives longest, which
# goes to its home once.

global @data : obj<256x8> = { zero 256 }

function @main() -> i32 [role=entry] {
  block ^entry:
    %base = addr @data
    %p0 = index i8 %base, 0
    %v0 = load f64 %p0
    %p1 = index i8 %base, 8
    %v1 = load f64 %p1
    %p2 = index i8 %base, 16
    %v2 = load f64 %p2
    %p3 = index i8 %base, 24
    %v3 = load f64 %p3
    %p4 = index i8 %base, 32
    %v4 = load f64 %p4
    %p5 = index i8 %base, 40
    %v5 = load f64 %p5
    %p6 = index i8 %base, 48
    %v6 = load f64 %p6
    %p7 = index i8 %base, 56
    %v7 = load f64 %p7
    %p8 = index i8 %base, 64
    %v8 = load f64 %p8
    %p9 = index i8 %base, 72
    %v9 = load f64 %p9
    %p10 = index i8 %base, 80
    %v10 = load f64 %p10
    %p11 = index i8 %base, 88
    %v11 = load f64 %p11
    %p12 = index i8 %base, 96
    %v12 = load f64 %p12
    %p13 = index i8 %base, 104
    %v13 = load f64 %p13
    %p14 = index i8 %base, 112
    %v14 = load f64 %p14
    %p15 = index i8 %base, 120
    %v15 = load f64 %p15
    %m1 = binary mul f64 %v0, %v1
    %a1 = binary add f64 %m1, %v1
    %m2 = binary mul f64 %a1, %v2
    %a2 = binary add f64 %m2, %v2
    %m3 = binary mul f64 %a2, %v3
    %a3 = binary add f64 %m3, %v3
    %m4 = binary mul f64 %a3, %v4
    %a4 = binary add f64 %m4, %v4
    %m5 = binary mul f64 %a4, %v5
    %a5 = binary add f64 %m5, %v5
    %m6 = binary mul f64 %a5, %v6
    %a6 = binary add f64 %m6, %v6
    %m7 = binary mul f64 %a6, %v7
    %a7 = binary add f64 %m7, %v7
    %m8 = binary mul f64 %a7, %v8
    %a8 = binary add f64 %m8, %v8
    %m9 = binary mul f64 %a8, %v9
    %a9 = binary add f64 %m9, %v9
    %m10 = binary mul f64 %a9, %v10
    %a10 = binary add f64 %m10, %v10
    %m11 = binary mul f64 %a10, %v11
    %a11 = binary add f64 %m11, %v11
    %m12 = binary mul f64 %a11, %v12
    %a12 = binary add f64 %m12, %v12
    %m13 = binary mul f64 %a12, %v13
    %a13 = binary add f64 %m13, %v13
    %m14 = binary mul f64 %a13, %v14
    %a14 = binary add f64 %m14, %v14
    %m15 = binary mul f64 %a14, %v15
    %a15 = binary add f64 %m15, %v15
    store f64 %v0, %p0
    store f64 %v1, %p1
    store f64 %v2, %p2
    store f64 %v3, %p3
    store f64 %v4, %p4
    store f64 %v5, %p5
    store f64 %v6, %p6
    store f64 %v7, %p7
    store f64 %v8, %p8
    store f64 %v9, %p9
    store f64 %v10, %p10
    store f64 %v11, %p11
    store f64 %v12, %p12
    store f64 %v13, %p13
    store f64 %v14, %p14
    store f64 %v15, %p15
    store f64 %a15, %base
    return i32 0
}
