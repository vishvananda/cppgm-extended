# A switch costs a Wasm site its range test and its table: this callee of one
# switch goes inline at both sites.
target wasm32 1

function @field(%p : ptr, %k : i64) -> i64 {
  block ^entry:
    switch %k, ^other, 0:^b0, 1:^b0, 2:^b0, 3:^b0, 4:^b0, 5:^b1, 6:^b1, 7:^b1, 8:^b1, 9:^b1, 10:^b2, 11:^b2, 12:^b2, 13:^b2, 14:^b2, 15:^b3, 16:^b3, 17:^b3, 18:^b3, 19:^b3, 20:^b4, 21:^b4, 22:^b4, 23:^b4, 24:^b4, 25:^b5, 26:^b5, 27:^b5, 28:^b5, 29:^b5, 30:^b6, 31:^b6, 32:^b6, 33:^b6, 34:^b6, 35:^b7, 36:^b7, 37:^b7, 38:^b7, 39:^b7
  block ^b0:
    %f0 = index i8 %p, 8
    %v0 = load typed i64 %f0
    return i64 %v0
  block ^b1:
    %f1 = index i8 %p, 16
    %v1 = load typed i64 %f1
    return i64 %v1
  block ^b2:
    %f2 = index i8 %p, 40
    %v2 = load typed i64 %f2
    return i64 %v2
  block ^b3:
    %f3 = index i8 %p, 80
    %v3 = load typed i64 %f3
    return i64 %v3
  block ^b4:
    %f4 = index i8 %p, 136
    %v4 = load typed i64 %f4
    return i64 %v4
  block ^b5:
    %f5 = index i8 %p, 208
    %v5 = load typed i64 %f5
    return i64 %v5
  block ^b6:
    %f6 = index i8 %p, 296
    %v6 = load typed i64 %f6
    return i64 %v6
  block ^b7:
    %f7 = index i8 %p, 400
    %v7 = load typed i64 %f7
    return i64 %v7
  block ^other:
    return i64 0
}

function @first(%p : ptr, %q : ptr) -> i64 {
  block ^entry:
    %k = load typed i64 %q
    %r = call i64 @field(%p, %k)
    %t = binary add i64 %r, 1
    return i64 %t
}

function @second(%p : ptr, %q : ptr) -> i64 {
  block ^entry:
    %k = load typed i64 %q
    %r = call i64 @field(%p, %k)
    %t = binary xor i64 %r, 5
    return i64 %t
}
