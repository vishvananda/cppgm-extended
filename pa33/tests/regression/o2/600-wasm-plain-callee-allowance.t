# On Wasm a callee without the inline hint may cost its site more than on a
# native target, where a call costs little more than its instructions: this
# callee is too large for a native site and goes inline at both sites here.
target wasm32 1

function @mix(%p : ptr, %x : i64) -> i64 {
  block ^entry:
    %f0 = index i8 %p, 8
    %v0 = load typed i64 %f0
    %s0 = binary add i64 %x, %v0
    %f1 = index i8 %p, 16
    %v1 = load typed i64 %f1
    %s1 = binary xor i64 %s0, %v1
    %f2 = index i8 %p, 24
    %v2 = load typed i64 %f2
    %s2 = binary add i64 %s1, %v2
    %f3 = index i8 %p, 32
    %v3 = load typed i64 %f3
    %s3 = binary xor i64 %s2, %v3
    %f4 = index i8 %p, 40
    %v4 = load typed i64 %f4
    %s4 = binary add i64 %s3, %v4
    %f5 = index i8 %p, 48
    %v5 = load typed i64 %f5
    %s5 = binary xor i64 %s4, %v5
    %f6 = index i8 %p, 56
    %v6 = load typed i64 %f6
    %s6 = binary add i64 %s5, %v6
    %f7 = index i8 %p, 64
    %v7 = load typed i64 %f7
    %s7 = binary xor i64 %s6, %v7
    %f8 = index i8 %p, 72
    %v8 = load typed i64 %f8
    %s8 = binary add i64 %s7, %v8
    %f9 = index i8 %p, 80
    %v9 = load typed i64 %f9
    %s9 = binary xor i64 %s8, %v9
    %f10 = index i8 %p, 88
    %v10 = load typed i64 %f10
    %s10 = binary add i64 %s9, %v10
    %f11 = index i8 %p, 96
    %v11 = load typed i64 %f11
    %s11 = binary xor i64 %s10, %v11
    %f12 = index i8 %p, 104
    %v12 = load typed i64 %f12
    %s12 = binary add i64 %s11, %v12
    %f13 = index i8 %p, 112
    %v13 = load typed i64 %f13
    %s13 = binary xor i64 %s12, %v13
    %f14 = index i8 %p, 120
    %v14 = load typed i64 %f14
    %s14 = binary add i64 %s13, %v14
    %f15 = index i8 %p, 128
    %v15 = load typed i64 %f15
    %s15 = binary xor i64 %s14, %v15
    %f16 = index i8 %p, 136
    %v16 = load typed i64 %f16
    %s16 = binary add i64 %s15, %v16
    %f17 = index i8 %p, 144
    %v17 = load typed i64 %f17
    %s17 = binary xor i64 %s16, %v17
    %f18 = index i8 %p, 152
    %v18 = load typed i64 %f18
    %s18 = binary add i64 %s17, %v18
    %f19 = index i8 %p, 160
    %v19 = load typed i64 %f19
    %s19 = binary xor i64 %s18, %v19
    %f20 = index i8 %p, 168
    %v20 = load typed i64 %f20
    %s20 = binary add i64 %s19, %v20
    %f21 = index i8 %p, 176
    %v21 = load typed i64 %f21
    %s21 = binary xor i64 %s20, %v21
    %f22 = index i8 %p, 184
    %v22 = load typed i64 %f22
    %s22 = binary add i64 %s21, %v22
    %f23 = index i8 %p, 192
    %v23 = load typed i64 %f23
    %s23 = binary xor i64 %s22, %v23
    %f24 = index i8 %p, 200
    %v24 = load typed i64 %f24
    %s24 = binary add i64 %s23, %v24
    return i64 %s24
}

function @first(%p : ptr, %x : i64) -> i64 {
  block ^entry:
    %r = call i64 @mix(%p, %x)
    %t = binary add i64 %r, 1
    return i64 %t
}

function @second(%p : ptr, %x : i64) -> i64 {
  block ^entry:
    %r = call i64 @mix(%p, %x)
    %t = binary xor i64 %r, 5
    return i64 %t
}
