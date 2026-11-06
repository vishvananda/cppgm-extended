global @observed : i64 = 0

function @fold_u8(%x : u8) -> u8 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and u8 %x, 1
    %cond0 = cmp ne u8 %bit0, 0
    %s0 = binary ushr u8 %x, 1
    %m0 = binary xor u8 %s0, 184
    %r0 = select u8 %cond0, %s0, %m0
    %bit1 = binary and u8 %r0, 1
    %cond1 = cmp eq u8 %bit1, 1
    %s1 = binary ushr u8 %r0, 1
    %m1 = binary xor u8 %s1, 184
    %r1 = select u8 %cond1, %m1, %s1
    %bit2 = binary and u8 %r1, 1
    %cond2 = cmp eq u8 %bit2, 0
    %s2 = binary ushr u8 %r1, 1
    %m2 = binary xor u8 %s2, 184
    %r2 = select u8 %cond2, %m2, %s2
    %bit3 = binary and u8 %r2, 1
    %cond3 = cmp ne u8 %bit3, 0
    %s3 = binary ushr u8 %r2, 1
    %m3 = binary xor u8 %s3, 184
    %r3 = select u8 %cond3, %m3, %s3
    %bit4 = binary and u8 %r3, 1
    %cond4 = cmp eq u8 %bit4, 1
    %s4 = binary ushr u8 %r3, 1
    %m4 = binary xor u8 %s4, 184
    %r4 = select u8 %cond4, %s4, %m4
    %bit5 = binary and u8 %r4, 1
    %cond5 = cmp eq u8 %bit5, 0
    %s5 = binary ushr u8 %r4, 1
    %m5 = binary xor u8 %s5, 184
    %r5 = select u8 %cond5, %s5, %m5
    %bit6 = binary and u8 %r5, 1
    %cond6 = cmp ne u8 %bit6, 0
    %s6 = binary ushr u8 %r5, 1
    %m6 = binary xor u8 %s6, 184
    %r6 = select u8 %cond6, %s6, %m6
    %bit7 = binary and u8 %r6, 1
    %cond7 = cmp eq u8 %bit7, 1
    %s7 = binary ushr u8 %r6, 1
    %m7 = binary xor u8 %s7, 184
    %r7 = select u8 %cond7, %m7, %s7
    return u8 %r7
}

function @fold_i8(%x : i8) -> i8 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and i8 %x, 1
    %cond0 = cmp ne i8 %bit0, 0
    %s0 = binary ushr i8 %x, 1
    %m0 = binary xor i8 %s0, -72
    %r0 = select i8 %cond0, %s0, %m0
    %bit1 = binary and i8 %r0, 1
    %cond1 = cmp eq i8 %bit1, 1
    %s1 = binary ushr i8 %r0, 1
    %m1 = binary xor i8 %s1, -72
    %r1 = select i8 %cond1, %m1, %s1
    %bit2 = binary and i8 %r1, 1
    %cond2 = cmp eq i8 %bit2, 0
    %s2 = binary ushr i8 %r1, 1
    %m2 = binary xor i8 %s2, -72
    %r2 = select i8 %cond2, %m2, %s2
    %bit3 = binary and i8 %r2, 1
    %cond3 = cmp ne i8 %bit3, 0
    %s3 = binary ushr i8 %r2, 1
    %m3 = binary xor i8 %s3, -72
    %r3 = select i8 %cond3, %m3, %s3
    %bit4 = binary and i8 %r3, 1
    %cond4 = cmp eq i8 %bit4, 1
    %s4 = binary ushr i8 %r3, 1
    %m4 = binary xor i8 %s4, -72
    %r4 = select i8 %cond4, %s4, %m4
    %bit5 = binary and i8 %r4, 1
    %cond5 = cmp eq i8 %bit5, 0
    %s5 = binary ushr i8 %r4, 1
    %m5 = binary xor i8 %s5, -72
    %r5 = select i8 %cond5, %s5, %m5
    %bit6 = binary and i8 %r5, 1
    %cond6 = cmp ne i8 %bit6, 0
    %s6 = binary ushr i8 %r5, 1
    %m6 = binary xor i8 %s6, -72
    %r6 = select i8 %cond6, %s6, %m6
    %bit7 = binary and i8 %r6, 1
    %cond7 = cmp eq i8 %bit7, 1
    %s7 = binary ushr i8 %r6, 1
    %m7 = binary xor i8 %s7, -72
    %r7 = select i8 %cond7, %m7, %s7
    return i8 %r7
}

function @fold_u16(%x : u16) -> u16 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and u16 %x, 1
    %cond0 = cmp ne u16 %bit0, 0
    %s0 = binary ushr u16 %x, 1
    %m0 = binary xor u16 %s0, 46081
    %r0 = select u16 %cond0, %m0, %s0
    %bit1 = binary and u16 %r0, 1
    %cond1 = cmp eq u16 %bit1, 1
    %s1 = binary ushr u16 %r0, 1
    %m1 = binary xor u16 %s1, 46081
    %r1 = select u16 %cond1, %m1, %s1
    %bit2 = binary and u16 %r1, 1
    %cond2 = cmp eq u16 %bit2, 0
    %s2 = binary ushr u16 %r1, 1
    %m2 = binary xor u16 %s2, 46081
    %r2 = select u16 %cond2, %s2, %m2
    %bit3 = binary and u16 %r2, 1
    %cond3 = cmp ne u16 %bit3, 0
    %s3 = binary ushr u16 %r2, 1
    %m3 = binary xor u16 %s3, 46081
    %r3 = select u16 %cond3, %m3, %s3
    %bit4 = binary and u16 %r3, 1
    %cond4 = cmp eq u16 %bit4, 1
    %s4 = binary ushr u16 %r3, 1
    %m4 = binary xor u16 %s4, 46081
    %r4 = select u16 %cond4, %m4, %s4
    return u16 %r4
}

function @fold_u32(%x : u32) -> u32 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and u32 %x, 1
    %cond0 = cmp ne u32 %bit0, 0
    %s0 = binary ushr u32 %x, 1
    %m0 = binary xor u32 %s0, 2197175160
    %r0 = select u32 %cond0, %m0, %s0
    %bit1 = binary and u32 %r0, 1
    %cond1 = cmp eq u32 %bit1, 1
    %s1 = binary ushr u32 %r0, 1
    %m1 = binary xor u32 %s1, 2197175160
    %r1 = select u32 %cond1, %m1, %s1
    %bit2 = binary and u32 %r1, 1
    %cond2 = cmp eq u32 %bit2, 0
    %s2 = binary ushr u32 %r1, 1
    %m2 = binary xor u32 %s2, 2197175160
    %r2 = select u32 %cond2, %s2, %m2
    %bit3 = binary and u32 %r2, 1
    %cond3 = cmp ne u32 %bit3, 0
    %s3 = binary ushr u32 %r2, 1
    %m3 = binary xor u32 %s3, 2197175160
    %r3 = select u32 %cond3, %m3, %s3
    %bit4 = binary and u32 %r3, 1
    %cond4 = cmp eq u32 %bit4, 1
    %s4 = binary ushr u32 %r3, 1
    %m4 = binary xor u32 %s4, 2197175160
    %r4 = select u32 %cond4, %m4, %s4
    %bit5 = binary and u32 %r4, 1
    %cond5 = cmp eq u32 %bit5, 0
    %s5 = binary ushr u32 %r4, 1
    %m5 = binary xor u32 %s5, 2197175160
    %r5 = select u32 %cond5, %s5, %m5
    %bit6 = binary and u32 %r5, 1
    %cond6 = cmp ne u32 %bit6, 0
    %s6 = binary ushr u32 %r5, 1
    %m6 = binary xor u32 %s6, 2197175160
    %r6 = select u32 %cond6, %m6, %s6
    %bit7 = binary and u32 %r6, 1
    %cond7 = cmp eq u32 %bit7, 1
    branch %cond7, ^yes, ^no
  block ^yes:
    %sy = binary ushr u32 %r6, 1
    %my = binary xor u32 %sy, 2197175160
    jump ^merge
  block ^no:
    %sn = binary ushr u32 %r6, 1
    jump ^merge
  block ^merge:
    %r7 = phi u32 [^yes: %my, ^no: %sn]
    return u32 %r7
}

function @fold_u32_flip(%x : u32) -> u32 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and u32 %x, 1
    %cond0 = cmp ne u32 %bit0, 0
    %s0 = binary ushr u32 %x, 1
    %m0 = binary xor u32 %s0, 2813935905
    %r0 = select u32 %cond0, %s0, %m0
    %bit1 = binary and u32 %r0, 1
    %cond1 = cmp eq u32 %bit1, 1
    %s1 = binary ushr u32 %r0, 1
    %m1 = binary xor u32 %s1, 2813935905
    %r1 = select u32 %cond1, %m1, %s1
    %bit2 = binary and u32 %r1, 1
    %cond2 = cmp eq u32 %bit2, 0
    %s2 = binary ushr u32 %r1, 1
    %m2 = binary xor u32 %s2, 2813935905
    %r2 = select u32 %cond2, %m2, %s2
    %bit3 = binary and u32 %r2, 1
    %cond3 = cmp ne u32 %bit3, 0
    branch %cond3, ^yes, ^no
  block ^yes:
    %sy = binary ushr u32 %r2, 1
    %my = binary xor u32 %sy, 2813935905
    jump ^merge
  block ^no:
    %sn = binary ushr u32 %r2, 1
    jump ^merge
  block ^merge:
    %r3 = phi u32 [^yes: %my, ^no: %sn]
    return u32 %r3
}

function @fold_i64(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and i64 %x, 1
    %cond0 = cmp ne i64 %bit0, 0
    %s0 = binary ushr i64 %x, 1
    %m0 = binary xor i64 %s0, -2882303761517117440
    %r0 = select i64 %cond0, %m0, %s0
    %bit1 = binary and i64 %r0, 1
    %cond1 = cmp eq i64 %bit1, 1
    %s1 = binary ushr i64 %r0, 1
    %m1 = binary xor i64 %s1, -2882303761517117440
    %r1 = select i64 %cond1, %m1, %s1
    %bit2 = binary and i64 %r1, 1
    %cond2 = cmp eq i64 %bit2, 0
    %s2 = binary ushr i64 %r1, 1
    %m2 = binary xor i64 %s2, -2882303761517117440
    %r2 = select i64 %cond2, %s2, %m2
    %bit3 = binary and i64 %r2, 1
    %cond3 = cmp ne i64 %bit3, 0
    %s3 = binary ushr i64 %r2, 1
    %m3 = binary xor i64 %s3, -2882303761517117440
    %r3 = select i64 %cond3, %m3, %s3
    %bit4 = binary and i64 %r3, 1
    %cond4 = cmp eq i64 %bit4, 1
    %s4 = binary ushr i64 %r3, 1
    %m4 = binary xor i64 %s4, -2882303761517117440
    %r4 = select i64 %cond4, %m4, %s4
    %bit5 = binary and i64 %r4, 1
    %cond5 = cmp eq i64 %bit5, 0
    %s5 = binary ushr i64 %r4, 1
    %m5 = binary xor i64 %s5, -2882303761517117440
    %r5 = select i64 %cond5, %s5, %m5
    return i64 %r5
}

function @fold_i64_eight(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and i64 %x, 1
    %cond0 = cmp ne i64 %bit0, 0
    %s0 = binary ushr i64 %x, 1
    %m0 = binary xor i64 %s0, -3932672073523589310
    %r0 = select i64 %cond0, %m0, %s0
    %bit1 = binary and i64 %r0, 1
    %cond1 = cmp eq i64 %bit1, 1
    %s1 = binary ushr i64 %r0, 1
    %m1 = binary xor i64 %s1, -3932672073523589310
    %r1 = select i64 %cond1, %m1, %s1
    %bit2 = binary and i64 %r1, 1
    %cond2 = cmp eq i64 %bit2, 0
    %s2 = binary ushr i64 %r1, 1
    %m2 = binary xor i64 %s2, -3932672073523589310
    %r2 = select i64 %cond2, %s2, %m2
    %bit3 = binary and i64 %r2, 1
    %cond3 = cmp ne i64 %bit3, 0
    %s3 = binary ushr i64 %r2, 1
    %m3 = binary xor i64 %s3, -3932672073523589310
    %r3 = select i64 %cond3, %m3, %s3
    %bit4 = binary and i64 %r3, 1
    %cond4 = cmp eq i64 %bit4, 1
    %s4 = binary ushr i64 %r3, 1
    %m4 = binary xor i64 %s4, -3932672073523589310
    %r4 = select i64 %cond4, %m4, %s4
    %bit5 = binary and i64 %r4, 1
    %cond5 = cmp eq i64 %bit5, 0
    %s5 = binary ushr i64 %r4, 1
    %m5 = binary xor i64 %s5, -3932672073523589310
    %r5 = select i64 %cond5, %s5, %m5
    %bit6 = binary and i64 %r5, 1
    %cond6 = cmp ne i64 %bit6, 0
    %s6 = binary ushr i64 %r5, 1
    %m6 = binary xor i64 %s6, -3932672073523589310
    %r6 = select i64 %cond6, %m6, %s6
    %bit7 = binary and i64 %r6, 1
    %cond7 = cmp eq i64 %bit7, 1
    branch %cond7, ^yes, ^no
  block ^yes:
    %sy = binary ushr i64 %r6, 1
    %my = binary xor i64 %sy, -3932672073523589310
    jump ^merge
  block ^no:
    %sn = binary ushr i64 %r6, 1
    jump ^merge
  block ^merge:
    %r7 = phi i64 [^yes: %my, ^no: %sn]
    return i64 %r7
}

function @fold_sixteen(%x : u32) -> u32 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and u32 %x, 1
    %cond0 = cmp ne u32 %bit0, 0
    %s0 = binary ushr u32 %x, 1
    %m0 = binary xor u32 %s0, 2197175160
    %r0 = select u32 %cond0, %m0, %s0
    %bit1 = binary and u32 %r0, 1
    %cond1 = cmp eq u32 %bit1, 1
    %s1 = binary ushr u32 %r0, 1
    %m1 = binary xor u32 %s1, 2197175160
    %r1 = select u32 %cond1, %m1, %s1
    %bit2 = binary and u32 %r1, 1
    %cond2 = cmp eq u32 %bit2, 0
    %s2 = binary ushr u32 %r1, 1
    %m2 = binary xor u32 %s2, 2197175160
    %r2 = select u32 %cond2, %s2, %m2
    %bit3 = binary and u32 %r2, 1
    %cond3 = cmp ne u32 %bit3, 0
    %s3 = binary ushr u32 %r2, 1
    %m3 = binary xor u32 %s3, 2197175160
    %r3 = select u32 %cond3, %m3, %s3
    %bit4 = binary and u32 %r3, 1
    %cond4 = cmp eq u32 %bit4, 1
    %s4 = binary ushr u32 %r3, 1
    %m4 = binary xor u32 %s4, 2197175160
    %r4 = select u32 %cond4, %m4, %s4
    %bit5 = binary and u32 %r4, 1
    %cond5 = cmp eq u32 %bit5, 0
    %s5 = binary ushr u32 %r4, 1
    %m5 = binary xor u32 %s5, 2197175160
    %r5 = select u32 %cond5, %s5, %m5
    %bit6 = binary and u32 %r5, 1
    %cond6 = cmp ne u32 %bit6, 0
    %s6 = binary ushr u32 %r5, 1
    %m6 = binary xor u32 %s6, 2197175160
    %r6 = select u32 %cond6, %m6, %s6
    %bit7 = binary and u32 %r6, 1
    %cond7 = cmp eq u32 %bit7, 1
    %s7 = binary ushr u32 %r6, 1
    %m7 = binary xor u32 %s7, 2197175160
    %r7 = select u32 %cond7, %m7, %s7
    %bit8 = binary and u32 %r7, 1
    %cond8 = cmp eq u32 %bit8, 0
    %s8 = binary ushr u32 %r7, 1
    %m8 = binary xor u32 %s8, 2197175160
    %r8 = select u32 %cond8, %s8, %m8
    %bit9 = binary and u32 %r8, 1
    %cond9 = cmp ne u32 %bit9, 0
    %s9 = binary ushr u32 %r8, 1
    %m9 = binary xor u32 %s9, 2197175160
    %r9 = select u32 %cond9, %m9, %s9
    %bit10 = binary and u32 %r9, 1
    %cond10 = cmp eq u32 %bit10, 1
    %s10 = binary ushr u32 %r9, 1
    %m10 = binary xor u32 %s10, 2197175160
    %r10 = select u32 %cond10, %m10, %s10
    %bit11 = binary and u32 %r10, 1
    %cond11 = cmp eq u32 %bit11, 0
    %s11 = binary ushr u32 %r10, 1
    %m11 = binary xor u32 %s11, 2197175160
    %r11 = select u32 %cond11, %s11, %m11
    %bit12 = binary and u32 %r11, 1
    %cond12 = cmp ne u32 %bit12, 0
    %s12 = binary ushr u32 %r11, 1
    %m12 = binary xor u32 %s12, 2197175160
    %r12 = select u32 %cond12, %m12, %s12
    %bit13 = binary and u32 %r12, 1
    %cond13 = cmp eq u32 %bit13, 1
    %s13 = binary ushr u32 %r12, 1
    %m13 = binary xor u32 %s13, 2197175160
    %r13 = select u32 %cond13, %m13, %s13
    %bit14 = binary and u32 %r13, 1
    %cond14 = cmp eq u32 %bit14, 0
    %s14 = binary ushr u32 %r13, 1
    %m14 = binary xor u32 %s14, 2197175160
    %r14 = select u32 %cond14, %s14, %m14
    %bit15 = binary and u32 %r14, 1
    %cond15 = cmp ne u32 %bit15, 0
    %s15 = binary ushr u32 %r14, 1
    %m15 = binary xor u32 %s15, 2197175160
    %r15 = select u32 %cond15, %m15, %s15
    return u32 %r15
}

function @variable_poly(%x : u32, %p : u32) -> u32 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and u32 %x, 1
    %cond0 = cmp ne u32 %bit0, 0
    %s0 = binary ushr u32 %x, 1
    %m0 = binary xor u32 %s0, %p
    %r0 = select u32 %cond0, %m0, %s0
    %bit1 = binary and u32 %r0, 1
    %cond1 = cmp eq u32 %bit1, 1
    %s1 = binary ushr u32 %r0, 1
    %m1 = binary xor u32 %s1, %p
    %r1 = select u32 %cond1, %m1, %s1
    %bit2 = binary and u32 %r1, 1
    %cond2 = cmp eq u32 %bit2, 0
    %s2 = binary ushr u32 %r1, 1
    %m2 = binary xor u32 %s2, %p
    %r2 = select u32 %cond2, %s2, %m2
    %bit3 = binary and u32 %r2, 1
    %cond3 = cmp ne u32 %bit3, 0
    %s3 = binary ushr u32 %r2, 1
    %m3 = binary xor u32 %s3, %p
    %r3 = select u32 %cond3, %m3, %s3
    %bit4 = binary and u32 %r3, 1
    %cond4 = cmp eq u32 %bit4, 1
    %s4 = binary ushr u32 %r3, 1
    %m4 = binary xor u32 %s4, %p
    %r4 = select u32 %cond4, %m4, %s4
    %bit5 = binary and u32 %r4, 1
    %cond5 = cmp eq u32 %bit5, 0
    %s5 = binary ushr u32 %r4, 1
    %m5 = binary xor u32 %s5, %p
    %r5 = select u32 %cond5, %s5, %m5
    %bit6 = binary and u32 %r5, 1
    %cond6 = cmp ne u32 %bit6, 0
    %s6 = binary ushr u32 %r5, 1
    %m6 = binary xor u32 %s6, %p
    %r6 = select u32 %cond6, %m6, %s6
    %bit7 = binary and u32 %r6, 1
    %cond7 = cmp eq u32 %bit7, 1
    %s7 = binary ushr u32 %r6, 1
    %m7 = binary xor u32 %s7, %p
    %r7 = select u32 %cond7, %m7, %s7
    return u32 %r7
}

function @shift_two(%x : u32) -> u32 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and u32 %x, 1
    %cond0 = cmp ne u32 %bit0, 0
    %s0 = binary ushr u32 %x, 2
    %m0 = binary xor u32 %s0, 2813935905
    %r0 = select u32 %cond0, %m0, %s0
    %bit1 = binary and u32 %r0, 1
    %cond1 = cmp eq u32 %bit1, 1
    %s1 = binary ushr u32 %r0, 2
    %m1 = binary xor u32 %s1, 2813935905
    %r1 = select u32 %cond1, %m1, %s1
    %bit2 = binary and u32 %r1, 1
    %cond2 = cmp eq u32 %bit2, 0
    %s2 = binary ushr u32 %r1, 2
    %m2 = binary xor u32 %s2, 2813935905
    %r2 = select u32 %cond2, %s2, %m2
    %bit3 = binary and u32 %r2, 1
    %cond3 = cmp ne u32 %bit3, 0
    %s3 = binary ushr u32 %r2, 2
    %m3 = binary xor u32 %s3, 2813935905
    %r3 = select u32 %cond3, %m3, %s3
    %bit4 = binary and u32 %r3, 1
    %cond4 = cmp eq u32 %bit4, 1
    %s4 = binary ushr u32 %r3, 2
    %m4 = binary xor u32 %s4, 2813935905
    %r4 = select u32 %cond4, %m4, %s4
    %bit5 = binary and u32 %r4, 1
    %cond5 = cmp eq u32 %bit5, 0
    %s5 = binary ushr u32 %r4, 2
    %m5 = binary xor u32 %s5, 2813935905
    %r5 = select u32 %cond5, %s5, %m5
    %bit6 = binary and u32 %r5, 1
    %cond6 = cmp ne u32 %bit6, 0
    %s6 = binary ushr u32 %r5, 2
    %m6 = binary xor u32 %s6, 2813935905
    %r6 = select u32 %cond6, %m6, %s6
    %bit7 = binary and u32 %r6, 1
    %cond7 = cmp eq u32 %bit7, 1
    %s7 = binary ushr u32 %r6, 2
    %m7 = binary xor u32 %s7, 2813935905
    %r7 = select u32 %cond7, %m7, %s7
    return u32 %r7
}

function @arithmetic_shift(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and i32 %x, 1
    %cond0 = cmp ne i32 %bit0, 0
    %s0 = binary shr i32 %x, 1
    %m0 = binary xor i32 %s0, -1481031391
    %r0 = select i32 %cond0, %m0, %s0
    %bit1 = binary and i32 %r0, 1
    %cond1 = cmp eq i32 %bit1, 1
    %s1 = binary shr i32 %r0, 1
    %m1 = binary xor i32 %s1, -1481031391
    %r1 = select i32 %cond1, %m1, %s1
    %bit2 = binary and i32 %r1, 1
    %cond2 = cmp eq i32 %bit2, 0
    %s2 = binary shr i32 %r1, 1
    %m2 = binary xor i32 %s2, -1481031391
    %r2 = select i32 %cond2, %s2, %m2
    %bit3 = binary and i32 %r2, 1
    %cond3 = cmp ne i32 %bit3, 0
    %s3 = binary shr i32 %r2, 1
    %m3 = binary xor i32 %s3, -1481031391
    %r3 = select i32 %cond3, %m3, %s3
    %bit4 = binary and i32 %r3, 1
    %cond4 = cmp eq i32 %bit4, 1
    %s4 = binary shr i32 %r3, 1
    %m4 = binary xor i32 %s4, -1481031391
    %r4 = select i32 %cond4, %m4, %s4
    %bit5 = binary and i32 %r4, 1
    %cond5 = cmp eq i32 %bit5, 0
    %s5 = binary shr i32 %r4, 1
    %m5 = binary xor i32 %s5, -1481031391
    %r5 = select i32 %cond5, %s5, %m5
    %bit6 = binary and i32 %r5, 1
    %cond6 = cmp ne i32 %bit6, 0
    %s6 = binary shr i32 %r5, 1
    %m6 = binary xor i32 %s6, -1481031391
    %r6 = select i32 %cond6, %m6, %s6
    %bit7 = binary and i32 %r6, 1
    %cond7 = cmp eq i32 %bit7, 1
    %s7 = binary shr i32 %r6, 1
    %m7 = binary xor i32 %s7, -1481031391
    %r7 = select i32 %cond7, %m7, %s7
    return i32 %r7
}

function @external_uses(%x : u32) -> u32 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and u32 %x, 1
    %cond0 = cmp ne u32 %bit0, 0
    %s0 = binary ushr u32 %x, 1
    %m0 = binary xor u32 %s0, 2813935905
    %r0 = select u32 %cond0, %m0, %s0
    %wide0 = convert zext i64 u32 %r0
    %old0 = load volatile i64 @observed
    %sum0 = binary add i64 %old0, %wide0
    store volatile i64 %sum0, @observed
    %bit1 = binary and u32 %r0, 1
    %cond1 = cmp eq u32 %bit1, 1
    %s1 = binary ushr u32 %r0, 1
    %m1 = binary xor u32 %s1, 2813935905
    %r1 = select u32 %cond1, %m1, %s1
    %wide1 = convert zext i64 u32 %r1
    %old1 = load volatile i64 @observed
    %sum1 = binary add i64 %old1, %wide1
    store volatile i64 %sum1, @observed
    %bit2 = binary and u32 %r1, 1
    %cond2 = cmp eq u32 %bit2, 0
    %s2 = binary ushr u32 %r1, 1
    %m2 = binary xor u32 %s2, 2813935905
    %r2 = select u32 %cond2, %s2, %m2
    %wide2 = convert zext i64 u32 %r2
    %old2 = load volatile i64 @observed
    %sum2 = binary add i64 %old2, %wide2
    store volatile i64 %sum2, @observed
    %bit3 = binary and u32 %r2, 1
    %cond3 = cmp ne u32 %bit3, 0
    %s3 = binary ushr u32 %r2, 1
    %m3 = binary xor u32 %s3, 2813935905
    %r3 = select u32 %cond3, %m3, %s3
    %wide3 = convert zext i64 u32 %r3
    %old3 = load volatile i64 @observed
    %sum3 = binary add i64 %old3, %wide3
    store volatile i64 %sum3, @observed
    %bit4 = binary and u32 %r3, 1
    %cond4 = cmp eq u32 %bit4, 1
    %s4 = binary ushr u32 %r3, 1
    %m4 = binary xor u32 %s4, 2813935905
    %r4 = select u32 %cond4, %m4, %s4
    %wide4 = convert zext i64 u32 %r4
    %old4 = load volatile i64 @observed
    %sum4 = binary add i64 %old4, %wide4
    store volatile i64 %sum4, @observed
    %bit5 = binary and u32 %r4, 1
    %cond5 = cmp eq u32 %bit5, 0
    %s5 = binary ushr u32 %r4, 1
    %m5 = binary xor u32 %s5, 2813935905
    %r5 = select u32 %cond5, %s5, %m5
    %wide5 = convert zext i64 u32 %r5
    %old5 = load volatile i64 @observed
    %sum5 = binary add i64 %old5, %wide5
    store volatile i64 %sum5, @observed
    %bit6 = binary and u32 %r5, 1
    %cond6 = cmp ne u32 %bit6, 0
    %s6 = binary ushr u32 %r5, 1
    %m6 = binary xor u32 %s6, 2813935905
    %r6 = select u32 %cond6, %m6, %s6
    %wide6 = convert zext i64 u32 %r6
    %old6 = load volatile i64 @observed
    %sum6 = binary add i64 %old6, %wide6
    store volatile i64 %sum6, @observed
    %bit7 = binary and u32 %r6, 1
    %cond7 = cmp eq u32 %bit7, 1
    %s7 = binary ushr u32 %r6, 1
    %m7 = binary xor u32 %s7, 2813935905
    %r7 = select u32 %cond7, %m7, %s7
    %wide7 = convert zext i64 u32 %r7
    %old7 = load volatile i64 @observed
    %sum7 = binary add i64 %old7, %wide7
    store volatile i64 %sum7, @observed
    return u32 %r7
}

function @another_i64(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %bit0 = binary and i64 %x, 1
    %cond0 = cmp ne i64 %bit0, 0
    %s0 = binary ushr i64 %x, 1
    %m0 = binary xor i64 %s0, -7661587058870466123
    %r0 = select i64 %cond0, %m0, %s0
    %bit1 = binary and i64 %r0, 1
    %cond1 = cmp eq i64 %bit1, 1
    %s1 = binary ushr i64 %r0, 1
    %m1 = binary xor i64 %s1, -7661587058870466123
    %r1 = select i64 %cond1, %m1, %s1
    %bit2 = binary and i64 %r1, 1
    %cond2 = cmp eq i64 %bit2, 0
    %s2 = binary ushr i64 %r1, 1
    %m2 = binary xor i64 %s2, -7661587058870466123
    %r2 = select i64 %cond2, %s2, %m2
    %bit3 = binary and i64 %r2, 1
    %cond3 = cmp ne i64 %bit3, 0
    %s3 = binary ushr i64 %r2, 1
    %m3 = binary xor i64 %s3, -7661587058870466123
    %r3 = select i64 %cond3, %m3, %s3
    %bit4 = binary and i64 %r3, 1
    %cond4 = cmp eq i64 %bit4, 1
    %s4 = binary ushr i64 %r3, 1
    %m4 = binary xor i64 %s4, -7661587058870466123
    %r4 = select i64 %cond4, %m4, %s4
    %bit5 = binary and i64 %r4, 1
    %cond5 = cmp eq i64 %bit5, 0
    %s5 = binary ushr i64 %r4, 1
    %m5 = binary xor i64 %s5, -7661587058870466123
    %r5 = select i64 %cond5, %s5, %m5
    %bit6 = binary and i64 %r5, 1
    %cond6 = cmp ne i64 %bit6, 0
    %s6 = binary ushr i64 %r5, 1
    %m6 = binary xor i64 %s6, -7661587058870466123
    %r6 = select i64 %cond6, %m6, %s6
    %bit7 = binary and i64 %r6, 1
    %cond7 = cmp eq i64 %bit7, 1
    %s7 = binary ushr i64 %r6, 1
    %m7 = binary xor i64 %s7, -7661587058870466123
    %r7 = select i64 %cond7, %m7, %s7
    return i64 %r7
}
