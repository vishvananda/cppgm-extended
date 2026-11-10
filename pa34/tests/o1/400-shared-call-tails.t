# Equal call-containing tails may share code. Different callees and signed
# division retain their own behavior; effects occur once for each activation.
global @calls : i64 = 0
function @effect(%x : i64) -> i64 {
  block ^entry:
    %old = load volatile i64 @calls
    %next = binary add i64 %old, 3
    store volatile i64 %next, @calls
    %result = binary xor i64 %x, 90
    return i64 %result
}
function @other(%x : i64) -> i64 {
  block ^entry:
    %old = load volatile i64 @calls
    %next = binary add i64 %old, 5
    store volatile i64 %next, @calls
    %result = binary xor i64 %x, 91
    return i64 %result
}
function @worker(%n : i64, %x : i64, %d : i64) -> i64 {
  block ^entry:
    switch %n, ^fallback, 0:^c0, 1:^c1, 2:^c2, 3:^c3, 4:^c4, 5:^c5, 6:^c6, 7:^c7
  block ^c0:
    %v0_0 = call i64 @effect(%x)
    %v0_1 = binary xor i64 %v0_0, 33
    %v0_2 = binary mul i64 %v0_1, 17
    %v0_3 = binary add i64 %v0_2, 11
    %v0_4 = binary ushr i64 %v0_3, 3
    %v0_5 = binary xor i64 %v0_4, %x
    %v0_6 = binary udiv i64 %v0_5, %d
    %v0_7 = binary mul i64 %v0_6, 13
    %v0_8 = binary add i64 %v0_7, 29
    %v0_9 = binary xor i64 %v0_8, 77
    %v0_10 = binary shl i64 %v0_9, 5
    %v0_11 = binary or i64 %v0_10, 7
    %v0_12 = binary add i64 %v0_11, %x
    %v0_13 = binary xor i64 %v0_12, 255
    return i64 %v0_13
  block ^c1:
    %v1_0 = call i64 @effect(%x)
    %v1_1 = binary xor i64 %v1_0, 33
    %v1_2 = binary mul i64 %v1_1, 17
    %v1_3 = binary add i64 %v1_2, 11
    %v1_4 = binary ushr i64 %v1_3, 3
    %v1_5 = binary xor i64 %v1_4, %x
    %v1_6 = binary udiv i64 %v1_5, %d
    %v1_7 = binary mul i64 %v1_6, 13
    %v1_8 = binary add i64 %v1_7, 29
    %v1_9 = binary xor i64 %v1_8, 77
    %v1_10 = binary shl i64 %v1_9, 5
    %v1_11 = binary or i64 %v1_10, 7
    %v1_12 = binary add i64 %v1_11, %x
    %v1_13 = binary xor i64 %v1_12, 255
    return i64 %v1_13
  block ^c2:
    %v2_0 = call i64 @effect(%x)
    %v2_1 = binary xor i64 %v2_0, 33
    %v2_2 = binary mul i64 %v2_1, 17
    %v2_3 = binary add i64 %v2_2, 11
    %v2_4 = binary ushr i64 %v2_3, 3
    %v2_5 = binary xor i64 %v2_4, %x
    %v2_6 = binary udiv i64 %v2_5, %d
    %v2_7 = binary mul i64 %v2_6, 13
    %v2_8 = binary add i64 %v2_7, 29
    %v2_9 = binary xor i64 %v2_8, 77
    %v2_10 = binary shl i64 %v2_9, 5
    %v2_11 = binary or i64 %v2_10, 7
    %v2_12 = binary add i64 %v2_11, %x
    %v2_13 = binary xor i64 %v2_12, 255
    return i64 %v2_13
  block ^c3:
    %v3_0 = call i64 @effect(%x)
    %v3_1 = binary xor i64 %v3_0, 33
    %v3_2 = binary mul i64 %v3_1, 17
    %v3_3 = binary add i64 %v3_2, 11
    %v3_4 = binary ushr i64 %v3_3, 3
    %v3_5 = binary xor i64 %v3_4, %x
    %v3_6 = binary udiv i64 %v3_5, %d
    %v3_7 = binary mul i64 %v3_6, 13
    %v3_8 = binary add i64 %v3_7, 29
    %v3_9 = binary xor i64 %v3_8, 77
    %v3_10 = binary shl i64 %v3_9, 5
    %v3_11 = binary or i64 %v3_10, 7
    %v3_12 = binary add i64 %v3_11, %x
    %v3_13 = binary xor i64 %v3_12, 255
    return i64 %v3_13
  block ^c4:
    %v4_0 = call i64 @effect(%x)
    %v4_1 = binary xor i64 %v4_0, 33
    %v4_2 = binary mul i64 %v4_1, 17
    %v4_3 = binary add i64 %v4_2, 11
    %v4_4 = binary ushr i64 %v4_3, 3
    %v4_5 = binary xor i64 %v4_4, %x
    %v4_6 = binary udiv i64 %v4_5, %d
    %v4_7 = binary mul i64 %v4_6, 13
    %v4_8 = binary add i64 %v4_7, 29
    %v4_9 = binary xor i64 %v4_8, 77
    %v4_10 = binary shl i64 %v4_9, 5
    %v4_11 = binary or i64 %v4_10, 7
    %v4_12 = binary add i64 %v4_11, %x
    %v4_13 = binary xor i64 %v4_12, 255
    return i64 %v4_13
  block ^c5:
    %v5_0 = call i64 @effect(%x)
    %v5_1 = binary xor i64 %v5_0, 33
    %v5_2 = binary mul i64 %v5_1, 17
    %v5_3 = binary add i64 %v5_2, 11
    %v5_4 = binary ushr i64 %v5_3, 3
    %v5_5 = binary xor i64 %v5_4, %x
    %v5_6 = binary udiv i64 %v5_5, %d
    %v5_7 = binary mul i64 %v5_6, 13
    %v5_8 = binary add i64 %v5_7, 29
    %v5_9 = binary xor i64 %v5_8, 77
    %v5_10 = binary shl i64 %v5_9, 5
    %v5_11 = binary or i64 %v5_10, 7
    %v5_12 = binary add i64 %v5_11, %x
    %v5_13 = binary xor i64 %v5_12, 255
    return i64 %v5_13
  block ^c6:
    %v6_0 = call i64 @other(%x)
    %v6_1 = binary xor i64 %v6_0, 33
    %v6_2 = binary mul i64 %v6_1, 17
    %v6_3 = binary add i64 %v6_2, 11
    %v6_4 = binary ushr i64 %v6_3, 3
    %v6_5 = binary xor i64 %v6_4, %x
    %v6_6 = binary udiv i64 %v6_5, %d
    %v6_7 = binary mul i64 %v6_6, 13
    %v6_8 = binary add i64 %v6_7, 29
    %v6_9 = binary xor i64 %v6_8, 77
    %v6_10 = binary shl i64 %v6_9, 5
    %v6_11 = binary or i64 %v6_10, 7
    %v6_12 = binary add i64 %v6_11, %x
    %v6_13 = binary xor i64 %v6_12, 255
    return i64 %v6_13
  block ^c7:
    %v7_0 = call i64 @effect(%x)
    %v7_1 = binary xor i64 %v7_0, 33
    %v7_2 = binary mul i64 %v7_1, 17
    %v7_3 = binary add i64 %v7_2, 11
    %v7_4 = binary ushr i64 %v7_3, 3
    %v7_5 = binary xor i64 %v7_4, %x
    %v7_6 = binary div i64 %v7_5, %d
    %v7_7 = binary mul i64 %v7_6, 13
    %v7_8 = binary add i64 %v7_7, 29
    %v7_9 = binary xor i64 %v7_8, 77
    %v7_10 = binary shl i64 %v7_9, 5
    %v7_11 = binary or i64 %v7_10, 7
    %v7_12 = binary add i64 %v7_11, %x
    %v7_13 = binary xor i64 %v7_12, 255
    return i64 %v7_13
  block ^fallback:
    return i64 999
}
function @main() -> i64 [role=entry] {
  block ^entry:
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^next: %inext]
    %state = phi i64 [^entry: 781331, ^next: %new_state]
    %calls = phi i64 [^entry: 0, ^next: %expected_calls]
    %product = binary mul i64 %state, 6364136223846793005
    %new_state = binary add i64 %product, 1
    %choice = binary umod i64 %i, 9
    %masked = binary and i64 %i, 63
    %divisor = binary add i64 %masked, 1
    %actual = call i64 @worker(%choice, %new_state, %divisor)
    %different = cmp eq i64 %choice, 6
    %signed = cmp eq i64 %choice, 7
    %fallback = cmp eq i64 %choice, 8
    %mask = select i64 %different, 91, 90
    %v0 = binary xor i64 %new_state, %mask
    %v1 = binary xor i64 %v0, 33
    %v2 = binary mul i64 %v1, 17
    %v3 = binary add i64 %v2, 11
    %v4 = binary ushr i64 %v3, 3
    %v5 = binary xor i64 %v4, %new_state
    %uq = binary udiv i64 %v5, %divisor
    %sq = binary div i64 %v5, %divisor
    %v6 = select i64 %signed, %sq, %uq
    %v7 = binary mul i64 %v6, 13
    %v8 = binary add i64 %v7, 29
    %v9 = binary xor i64 %v8, 77
    %v10 = binary shl i64 %v9, 5
    %v11 = binary or i64 %v10, 7
    %v12 = binary add i64 %v11, %new_state
    %v13 = binary xor i64 %v12, 255
    %expected = select i64 %fallback, 999, %v13
    %step = select i64 %different, 5, 3
    %increment = select i64 %fallback, 0, %step
    %expected_calls = binary add i64 %calls, %increment
    %actual_calls = load volatile i64 @calls
    %bad_result = cmp ne i64 %actual, %expected
    %bad_calls = cmp ne i64 %actual_calls, %expected_calls
    %bad = binary or i64 %bad_result, %bad_calls
    branch %bad, ^fail, ^next
  block ^next:
    %inext = binary add i64 %i, 1
    %more = cmp ne i64 %inext, 4096
    branch %more, ^loop, ^done
  block ^fail:
    return i64 1
  block ^done:
    return i64 0
}
