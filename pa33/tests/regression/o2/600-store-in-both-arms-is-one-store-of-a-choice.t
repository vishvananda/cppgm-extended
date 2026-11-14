# -O2: a store that both arms of a branch make through one address is one
# store of a choice of their values. Two addresses are one when their
# instructions agree and their literal operands have one value: the arms of
# @either each scale `%k` by four. In @merge each arm first loads the
# array's address: the loop passes take the load out of the loop, and the
# branch is a choice after them. In @apart the arms scale `%k` by four and
# by eight, and in @widths they store four bytes and eight: these branches
# stay.
global @cells : obj<96x8> = { zero 96 }
global @holders : obj<16x8> = { zero 16 }

function @either(%out : ptr, %k : i64, %flag : i64, %a : i32, %b : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    branch %flag, ^first, ^second

  block ^first:
    %at1 = binary mul i64 %k, 4
    %to1 = index inbounds i8 %out, %at1
    store typed i32 %a, %to1
    jump ^end

  block ^second:
    %at2 = binary mul i64 %k, 4
    %to2 = index inbounds i8 %out, %at2
    store typed i32 %b, %to2
    jump ^end

  block ^end:
    return void
}

function @merge(%arr : ptr [pass=by_address, object_bytes=8], %temp : ptr [pass=by_address, object_bytes=8], %left : i64, %mid : i64, %right : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %j0 = binary add nowrap i64 %mid, 1
    jump ^head

  block ^head:
    %i = phi i64 [^entry: %left, ^latch: %i2]
    %j = phi i64 [^entry: %j0, ^latch: %j2]
    %k = phi i64 [^entry: %left, ^latch: %k1]
    %left_more = cmp le i64 %i, %mid
    branch %left_more, ^right_test, ^done

  block ^right_test:
    %right_more = cmp le i64 %j, %right
    branch %right_more, ^body, ^done

  block ^body:
    %from = load typed ptr %temp
    %i_at = binary mul i64 %i, 4
    %i_from = index inbounds i8 %from, %i_at
    %x = load typed i32 %i_from
    %j_at = binary mul i64 %j, 4
    %j_from = index inbounds i8 %from, %j_at
    %y = load typed i32 %j_from
    %low = cmp le i32 %x, %y
    branch %low, ^take_left, ^take_right

  block ^take_left:
    %to1 = load typed ptr %arr
    %k_at1 = binary mul i64 %k, 4
    %k_to1 = index inbounds i8 %to1, %k_at1
    store typed i32 %x, %k_to1
    %i1 = binary add nowrap i64 %i, 1
    jump ^latch

  block ^take_right:
    %to2 = load typed ptr %arr
    %k_at2 = binary mul i64 %k, 4
    %k_to2 = index inbounds i8 %to2, %k_at2
    store typed i32 %y, %k_to2
    %j1 = binary add nowrap i64 %j, 1
    jump ^latch

  block ^latch:
    %i2 = phi i64 [^take_left: %i1, ^take_right: %i]
    %j2 = phi i64 [^take_left: %j, ^take_right: %j1]
    %k1 = binary add nowrap i64 %k, 1
    jump ^head

  block ^done:
    return i64 %k
}

function @apart(%out : ptr, %k : i64, %flag : i64, %v : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    branch %flag, ^first, ^second

  block ^first:
    %at1 = binary mul i64 %k, 4
    %to1 = index inbounds i8 %out, %at1
    store typed i32 %v, %to1
    jump ^end

  block ^second:
    %at2 = binary mul i64 %k, 8
    %to2 = index inbounds i8 %out, %at2
    store typed i32 %v, %to2
    jump ^end

  block ^end:
    return void
}

function @widths(%out : ptr, %k : i64, %flag : i64, %v : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    branch %flag, ^first, ^second

  block ^first:
    %at1 = binary mul i64 %k, 8
    %to1 = index inbounds i8 %out, %at1
    %narrow = convert trunc i32 i64 %v
    store typed i32 %narrow, %to1
    jump ^end

  block ^second:
    %at2 = binary mul i64 %k, 8
    %to2 = index inbounds i8 %out, %at2
    store typed i64 %v, %to2
    jump ^end

  block ^end:
    return void
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %cells = addr @cells
    %in = index i8 %cells, 64
    %holders = addr @holders
    %in_holder = index i8 %holders, 8
    store typed ptr %cells, %holders
    store typed ptr %in, %in_holder
    %in1 = index i8 %in, 8
    %in2 = index i8 %in, 16
    %in3 = index i8 %in, 24
    store i64 17179869185, %in
    store i64 38654705670, %in1
    store i64 12884901890, %in2
    store i64 34359738375, %in3
    %count = call i64 @merge(%holders, %in_holder, 0, 3, 7)
    %count_bad = cmp ne i64 %count, 7
    %out1 = index i8 %cells, 8
    %out2 = index i8 %cells, 16
    %out3 = index i8 %cells, 24
    %m0 = load i64 %cells
    %m1 = load i64 %out1
    %m2 = load i64 %out2
    %m3 = load i64 %out3
    %m0_bad = cmp ne i64 %m0, 8589934593
    %m1_bad = cmp ne i64 %m1, 17179869187
    %m2_bad = cmp ne i64 %m2, 30064771078
    %m3_bad = cmp ne i64 %m3, 8
    %spare = index i8 %cells, 32
    call void @either(%spare, 0, 1, 11, 12)
    call void @either(%spare, 1, 0, 11, 12)
    call void @apart(%spare, 2, 1, 13)
    call void @apart(%spare, 2, 0, 14)
    %e0 = load i64 %spare
    %e0_bad = cmp ne i64 %e0, 51539607563
    %spare1 = index i8 %spare, 8
    %spare2 = index i8 %spare, 16
    %e1 = load i64 %spare1
    %e2 = load i64 %spare2
    %e12 = binary shl i64 %e2, 8
    %e3 = binary or i64 %e1, %e12
    %e1_bad = cmp ne i64 %e3, 3597
    call void @widths(%spare, 1, 0, 4294967301)
    call void @widths(%spare, 1, 1, 9)
    %w = load i64 %spare1
    %w_bad = cmp ne i64 %w, 4294967305
    %bad0 = binary or i64 %count_bad, %m0_bad
    %bad1 = binary or i64 %m1_bad, %m2_bad
    %bad2 = binary or i64 %m3_bad, %e0_bad
    %bad3 = binary or i64 %e1_bad, %w_bad
    %bad4 = binary or i64 %bad0, %bad1
    %bad5 = binary or i64 %bad2, %bad3
    %bad = binary or i64 %bad4, %bad5
    return i64 %bad
}
