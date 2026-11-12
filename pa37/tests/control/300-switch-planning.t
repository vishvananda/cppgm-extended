# Dense and sparse dispatch retain full selector width and PHI edge values.
target wasm32 1

function @dense(%x : i64) -> i32 {
  block ^entry:
    switch %x, ^fallback, 0:^case0, 1:^case1, 2:^case2, 3:^case3, 4:^case4, 5:^case5, 6:^case6, 7:^case7, 8:^case8, 9:^case9, 10:^case10, 11:^case11, 12:^case12, 13:^case13, 14:^case14, 15:^case15, 16:^case16, 17:^case17, 18:^case18, 19:^case19, 20:^case20, 21:^case21, 22:^case22, 23:^case23, 24:^case24, 25:^case25, 26:^case26, 27:^case27, 28:^case28, 29:^case29, 30:^case30, 31:^case31, 32:^case32, 33:^case33, 34:^case34, 35:^case35, 36:^case36, 37:^case37, 38:^case38, 39:^case39, 40:^case40, 41:^case41, 42:^case42, 43:^case43, 44:^case44, 45:^case45, 46:^case46, 47:^case47, 48:^case48, 49:^case49, 50:^case50, 51:^case51, 52:^case52, 53:^case53, 54:^case54, 55:^case55, 56:^case56, 57:^case57, 58:^case58, 59:^case59, 60:^case60, 61:^case61, 62:^case62, 63:^case63
  block ^case0:
    jump ^exit
  block ^case1:
    jump ^exit
  block ^case2:
    jump ^exit
  block ^case3:
    jump ^exit
  block ^case4:
    jump ^exit
  block ^case5:
    jump ^exit
  block ^case6:
    jump ^exit
  block ^case7:
    jump ^exit
  block ^case8:
    jump ^exit
  block ^case9:
    jump ^exit
  block ^case10:
    jump ^exit
  block ^case11:
    jump ^exit
  block ^case12:
    jump ^exit
  block ^case13:
    jump ^exit
  block ^case14:
    jump ^exit
  block ^case15:
    jump ^exit
  block ^case16:
    jump ^exit
  block ^case17:
    jump ^exit
  block ^case18:
    jump ^exit
  block ^case19:
    jump ^exit
  block ^case20:
    jump ^exit
  block ^case21:
    jump ^exit
  block ^case22:
    jump ^exit
  block ^case23:
    jump ^exit
  block ^case24:
    jump ^exit
  block ^case25:
    jump ^exit
  block ^case26:
    jump ^exit
  block ^case27:
    jump ^exit
  block ^case28:
    jump ^exit
  block ^case29:
    jump ^exit
  block ^case30:
    jump ^exit
  block ^case31:
    jump ^exit
  block ^case32:
    jump ^exit
  block ^case33:
    jump ^exit
  block ^case34:
    jump ^exit
  block ^case35:
    jump ^exit
  block ^case36:
    jump ^exit
  block ^case37:
    jump ^exit
  block ^case38:
    jump ^exit
  block ^case39:
    jump ^exit
  block ^case40:
    jump ^exit
  block ^case41:
    jump ^exit
  block ^case42:
    jump ^exit
  block ^case43:
    jump ^exit
  block ^case44:
    jump ^exit
  block ^case45:
    jump ^exit
  block ^case46:
    jump ^exit
  block ^case47:
    jump ^exit
  block ^case48:
    jump ^exit
  block ^case49:
    jump ^exit
  block ^case50:
    jump ^exit
  block ^case51:
    jump ^exit
  block ^case52:
    jump ^exit
  block ^case53:
    jump ^exit
  block ^case54:
    jump ^exit
  block ^case55:
    jump ^exit
  block ^case56:
    jump ^exit
  block ^case57:
    jump ^exit
  block ^case58:
    jump ^exit
  block ^case59:
    jump ^exit
  block ^case60:
    jump ^exit
  block ^case61:
    jump ^exit
  block ^case62:
    jump ^exit
  block ^case63:
    jump ^exit
  block ^fallback:
    jump ^exit
  block ^exit:
    %result = phi i32 [^case0: 11, ^case1: 12, ^case2: 13, ^case3: 14, ^case4: 15, ^case5: 16, ^case6: 17, ^case7: 18, ^case8: 19, ^case9: 20, ^case10: 21, ^case11: 22, ^case12: 23, ^case13: 24, ^case14: 25, ^case15: 26, ^case16: 27, ^case17: 28, ^case18: 29, ^case19: 30, ^case20: 31, ^case21: 32, ^case22: 33, ^case23: 34, ^case24: 35, ^case25: 36, ^case26: 37, ^case27: 38, ^case28: 39, ^case29: 40, ^case30: 41, ^case31: 42, ^case32: 43, ^case33: 44, ^case34: 45, ^case35: 46, ^case36: 47, ^case37: 48, ^case38: 49, ^case39: 50, ^case40: 51, ^case41: 52, ^case42: 53, ^case43: 54, ^case44: 55, ^case45: 56, ^case46: 57, ^case47: 58, ^case48: 59, ^case49: 60, ^case50: 61, ^case51: 62, ^case52: 63, ^case53: 64, ^case54: 65, ^case55: 66, ^case56: 67, ^case57: 68, ^case58: 69, ^case59: 70, ^case60: 71, ^case61: 72, ^case62: 73, ^case63: 74, ^fallback: -1]
    return i32 %result
}

function @sparse(%x : i64) -> i32 {
  block ^entry:
    switch %x, ^fallback, 0:^case0, 7:^case1, 4294967303:^case2, 1000000000000:^case3, 9000000000000000000:^case4, 9223372036854775807:^case5, 9223372036854775808:^case6, 18446744073709551615:^case7
  block ^case0:
    jump ^exit
  block ^case1:
    jump ^exit
  block ^case2:
    jump ^exit
  block ^case3:
    jump ^exit
  block ^case4:
    jump ^exit
  block ^case5:
    jump ^exit
  block ^case6:
    jump ^exit
  block ^case7:
    jump ^exit
  block ^fallback:
    jump ^exit
  block ^exit:
    %result = phi i32 [^case0: 11, ^case1: 12, ^case2: 13, ^case3: 14, ^case4: 15, ^case5: 16, ^case6: 17, ^case7: 18, ^fallback: -1]
    return i32 %result
}

function @unsigned32(%x : u32) -> i32 {
  block ^entry:
    switch %x, ^fallback, 4294967290:^case0, 4294967291:^case1, 4294967292:^case2, 4294967293:^case3, 4294967294:^case4, 4294967295:^case5
  block ^case0:
    jump ^exit
  block ^case1:
    jump ^exit
  block ^case2:
    jump ^exit
  block ^case3:
    jump ^exit
  block ^case4:
    jump ^exit
  block ^case5:
    jump ^exit
  block ^fallback:
    jump ^exit
  block ^exit:
    %result = phi i32 [^case0: 11, ^case1: 12, ^case2: 13, ^case3: 14, ^case4: 15, ^case5: 16, ^fallback: -1]
    return i32 %result
}

function @signed32(%x : i32) -> i32 {
  block ^entry:
    switch %x, ^fallback, -4:^case0, -3:^case1, -2:^case2, -1:^case3, 0:^case4, 1:^case5
  block ^case0:
    jump ^exit
  block ^case1:
    jump ^exit
  block ^case2:
    jump ^exit
  block ^case3:
    jump ^exit
  block ^case4:
    jump ^exit
  block ^case5:
    jump ^exit
  block ^fallback:
    jump ^exit
  block ^exit:
    %result = phi i32 [^case0: 11, ^case1: 12, ^case2: 13, ^case3: 14, ^case4: 15, ^case5: 16, ^fallback: -1]
    return i32 %result
}

function @pointer_selector(%x : ptr) -> i32 {
  block ^entry:
    switch %x, ^fallback, 0:^zero, 1:^one, 2:^two, 3:^three
  block ^zero:
    return i32 10
  block ^one:
    return i32 11
  block ^two:
    return i32 12
  block ^three:
    return i32 13
  block ^fallback:
    return i32 -1
}

function @truth_selector(%x : i1) -> i32 {
  block ^entry:
    switch %x, ^fallback, 0:^zero, 1:^one, 2:^two, 3:^three
  block ^zero:
    return i32 10
  block ^one:
    return i32 11
  block ^two:
    return i32 12
  block ^three:
    return i32 13
  block ^fallback:
    return i32 -1
}
