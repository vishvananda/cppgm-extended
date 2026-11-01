# v128 phis across a call live in memory. A rotating set of them, and a phi that
# takes the literal 0, move all sixteen bytes between homes.
global @ticks : obj<4x4> = {
  i32 0
}

function @tick() -> void {
  block ^entry:
    %place = addr @ticks
    %old = load i32 %place
    %new = binary add i32 %old, 1
    store i32 %new, %place
    return void
}

function @rotate(%rounds : i32) -> i32 {
  block ^entry:
    %low0 = vector fromi32 1
    %start0 = vector pshufd %low0, 0
    %low1 = vector fromi32 2
    %start1 = vector pshufd %low1, 0
    %low2 = vector fromi32 3
    %start2 = vector pshufd %low2, 0
    %low3 = vector fromi32 4
    %start3 = vector pshufd %low3, 0
    %low4 = vector fromi32 5
    %start4 = vector pshufd %low4, 0
    %low5 = vector fromi32 6
    %start5 = vector pshufd %low5, 0
    %low6 = vector fromi32 7
    %start6 = vector pshufd %low6, 0
    %low7 = vector fromi32 8
    %start7 = vector pshufd %low7, 0
    %low8 = vector fromi32 9
    %start8 = vector pshufd %low8, 0
    %low9 = vector fromi32 10
    %start9 = vector pshufd %low9, 0
    %low10 = vector fromi32 11
    %start10 = vector pshufd %low10, 0
    %low11 = vector fromi32 12
    %start11 = vector pshufd %low11, 0
    %low12 = vector fromi32 13
    %start12 = vector pshufd %low12, 0
    %low13 = vector fromi32 14
    %start13 = vector pshufd %low13, 0
    %low14 = vector fromi32 15
    %start14 = vector pshufd %low14, 0
    %low15 = vector fromi32 16
    %start15 = vector pshufd %low15, 0
    %low16 = vector fromi32 17
    %start16 = vector pshufd %low16, 0
    %low17 = vector fromi32 18
    %start17 = vector pshufd %low17, 0
    %low18 = vector fromi32 19
    %start18 = vector pshufd %low18, 0
    %low19 = vector fromi32 20
    %start19 = vector pshufd %low19, 0
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^body: %next, ^again: 0]
    %limit = phi i32 [^entry: %rounds, ^body: %limit, ^again: 0]
    %v0 = phi v128 [^entry: %start0, ^body: %v1, ^again: %v0]
    %v1 = phi v128 [^entry: %start1, ^body: %v2, ^again: %v1]
    %v2 = phi v128 [^entry: %start2, ^body: %v3, ^again: %v2]
    %v3 = phi v128 [^entry: %start3, ^body: %v4, ^again: %v3]
    %v4 = phi v128 [^entry: %start4, ^body: %v5, ^again: %v4]
    %v5 = phi v128 [^entry: %start5, ^body: %v6, ^again: %v5]
    %v6 = phi v128 [^entry: %start6, ^body: %v7, ^again: %v6]
    %v7 = phi v128 [^entry: %start7, ^body: %v8, ^again: %v7]
    %v8 = phi v128 [^entry: %start8, ^body: %v9, ^again: %v8]
    %v9 = phi v128 [^entry: %start9, ^body: %v10, ^again: %v9]
    %v10 = phi v128 [^entry: %start10, ^body: %v11, ^again: %v10]
    %v11 = phi v128 [^entry: %start11, ^body: %v12, ^again: %v11]
    %v12 = phi v128 [^entry: %start12, ^body: %v13, ^again: %v12]
    %v13 = phi v128 [^entry: %start13, ^body: %v14, ^again: %v13]
    %v14 = phi v128 [^entry: %start14, ^body: %v15, ^again: %v14]
    %v15 = phi v128 [^entry: %start15, ^body: %v16, ^again: %v15]
    %v16 = phi v128 [^entry: %start16, ^body: %v17, ^again: %v16]
    %v17 = phi v128 [^entry: %start17, ^body: %v18, ^again: %v17]
    %v18 = phi v128 [^entry: %start18, ^body: %v19, ^again: %v18]
    %v19 = phi v128 [^entry: %start19, ^body: %v0, ^again: %v19]
    %zero = phi v128 [^entry: 0, ^body: %v3, ^again: 0]
    %more = cmp lt i32 %i, %limit
    branch %more, ^body, ^done

  block ^body:
    call void @tick()
    %next = binary add nowrap i32 %i, 1
    jump ^loop

  block ^done:
    call void @tick()
    branch %limit, ^again, ^check

  block ^again:
    call void @tick()
    jump ^loop

  block ^check:
    %zero_high = vector pshufd %zero, 3
    %zero_word = vector toi32 %zero_high
    %zero_low = vector toi32 %zero
    %sum0 = binary add i32 %zero_word, %zero_low
    %h0 = vector pshufd %v0, 3
    %word0 = vector toi32 %h0
    %low_word0 = vector toi32 %v0
    %pair0 = binary sub i32 %word0, %low_word0
    %scaled0 = binary mul i32 %word0, 1
    %step0 = binary add i32 %sum0, %scaled0
    %sum1 = binary add i32 %step0, %pair0
    %h1 = vector pshufd %v1, 3
    %word1 = vector toi32 %h1
    %low_word1 = vector toi32 %v1
    %pair1 = binary sub i32 %word1, %low_word1
    %scaled1 = binary mul i32 %word1, 2
    %step1 = binary add i32 %sum1, %scaled1
    %sum2 = binary add i32 %step1, %pair1
    %h2 = vector pshufd %v2, 3
    %word2 = vector toi32 %h2
    %low_word2 = vector toi32 %v2
    %pair2 = binary sub i32 %word2, %low_word2
    %scaled2 = binary mul i32 %word2, 3
    %step2 = binary add i32 %sum2, %scaled2
    %sum3 = binary add i32 %step2, %pair2
    %h3 = vector pshufd %v3, 3
    %word3 = vector toi32 %h3
    %low_word3 = vector toi32 %v3
    %pair3 = binary sub i32 %word3, %low_word3
    %scaled3 = binary mul i32 %word3, 4
    %step3 = binary add i32 %sum3, %scaled3
    %sum4 = binary add i32 %step3, %pair3
    %h4 = vector pshufd %v4, 3
    %word4 = vector toi32 %h4
    %low_word4 = vector toi32 %v4
    %pair4 = binary sub i32 %word4, %low_word4
    %scaled4 = binary mul i32 %word4, 5
    %step4 = binary add i32 %sum4, %scaled4
    %sum5 = binary add i32 %step4, %pair4
    %h5 = vector pshufd %v5, 3
    %word5 = vector toi32 %h5
    %low_word5 = vector toi32 %v5
    %pair5 = binary sub i32 %word5, %low_word5
    %scaled5 = binary mul i32 %word5, 6
    %step5 = binary add i32 %sum5, %scaled5
    %sum6 = binary add i32 %step5, %pair5
    %h6 = vector pshufd %v6, 3
    %word6 = vector toi32 %h6
    %low_word6 = vector toi32 %v6
    %pair6 = binary sub i32 %word6, %low_word6
    %scaled6 = binary mul i32 %word6, 7
    %step6 = binary add i32 %sum6, %scaled6
    %sum7 = binary add i32 %step6, %pair6
    %h7 = vector pshufd %v7, 3
    %word7 = vector toi32 %h7
    %low_word7 = vector toi32 %v7
    %pair7 = binary sub i32 %word7, %low_word7
    %scaled7 = binary mul i32 %word7, 8
    %step7 = binary add i32 %sum7, %scaled7
    %sum8 = binary add i32 %step7, %pair7
    %h8 = vector pshufd %v8, 3
    %word8 = vector toi32 %h8
    %low_word8 = vector toi32 %v8
    %pair8 = binary sub i32 %word8, %low_word8
    %scaled8 = binary mul i32 %word8, 9
    %step8 = binary add i32 %sum8, %scaled8
    %sum9 = binary add i32 %step8, %pair8
    %h9 = vector pshufd %v9, 3
    %word9 = vector toi32 %h9
    %low_word9 = vector toi32 %v9
    %pair9 = binary sub i32 %word9, %low_word9
    %scaled9 = binary mul i32 %word9, 10
    %step9 = binary add i32 %sum9, %scaled9
    %sum10 = binary add i32 %step9, %pair9
    %h10 = vector pshufd %v10, 3
    %word10 = vector toi32 %h10
    %low_word10 = vector toi32 %v10
    %pair10 = binary sub i32 %word10, %low_word10
    %scaled10 = binary mul i32 %word10, 11
    %step10 = binary add i32 %sum10, %scaled10
    %sum11 = binary add i32 %step10, %pair10
    %h11 = vector pshufd %v11, 3
    %word11 = vector toi32 %h11
    %low_word11 = vector toi32 %v11
    %pair11 = binary sub i32 %word11, %low_word11
    %scaled11 = binary mul i32 %word11, 12
    %step11 = binary add i32 %sum11, %scaled11
    %sum12 = binary add i32 %step11, %pair11
    %h12 = vector pshufd %v12, 3
    %word12 = vector toi32 %h12
    %low_word12 = vector toi32 %v12
    %pair12 = binary sub i32 %word12, %low_word12
    %scaled12 = binary mul i32 %word12, 13
    %step12 = binary add i32 %sum12, %scaled12
    %sum13 = binary add i32 %step12, %pair12
    %h13 = vector pshufd %v13, 3
    %word13 = vector toi32 %h13
    %low_word13 = vector toi32 %v13
    %pair13 = binary sub i32 %word13, %low_word13
    %scaled13 = binary mul i32 %word13, 14
    %step13 = binary add i32 %sum13, %scaled13
    %sum14 = binary add i32 %step13, %pair13
    %h14 = vector pshufd %v14, 3
    %word14 = vector toi32 %h14
    %low_word14 = vector toi32 %v14
    %pair14 = binary sub i32 %word14, %low_word14
    %scaled14 = binary mul i32 %word14, 15
    %step14 = binary add i32 %sum14, %scaled14
    %sum15 = binary add i32 %step14, %pair14
    %h15 = vector pshufd %v15, 3
    %word15 = vector toi32 %h15
    %low_word15 = vector toi32 %v15
    %pair15 = binary sub i32 %word15, %low_word15
    %scaled15 = binary mul i32 %word15, 16
    %step15 = binary add i32 %sum15, %scaled15
    %sum16 = binary add i32 %step15, %pair15
    %h16 = vector pshufd %v16, 3
    %word16 = vector toi32 %h16
    %low_word16 = vector toi32 %v16
    %pair16 = binary sub i32 %word16, %low_word16
    %scaled16 = binary mul i32 %word16, 17
    %step16 = binary add i32 %sum16, %scaled16
    %sum17 = binary add i32 %step16, %pair16
    %h17 = vector pshufd %v17, 3
    %word17 = vector toi32 %h17
    %low_word17 = vector toi32 %v17
    %pair17 = binary sub i32 %word17, %low_word17
    %scaled17 = binary mul i32 %word17, 18
    %step17 = binary add i32 %sum17, %scaled17
    %sum18 = binary add i32 %step17, %pair17
    %h18 = vector pshufd %v18, 3
    %word18 = vector toi32 %h18
    %low_word18 = vector toi32 %v18
    %pair18 = binary sub i32 %word18, %low_word18
    %scaled18 = binary mul i32 %word18, 19
    %step18 = binary add i32 %sum18, %scaled18
    %sum19 = binary add i32 %step18, %pair18
    %h19 = vector pshufd %v19, 3
    %word19 = vector toi32 %h19
    %low_word19 = vector toi32 %v19
    %pair19 = binary sub i32 %word19, %low_word19
    %scaled19 = binary mul i32 %word19, 20
    %step19 = binary add i32 %sum19, %scaled19
    %sum20 = binary add i32 %step19, %pair19
    return i32 %sum20
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %sum = call i32 @rotate(7)
    %right = cmp eq i32 %sum, 1960
    branch %right, ^pass, ^fail

  block ^pass:
    return i32 0

  block ^fail:
    return i32 1
}
