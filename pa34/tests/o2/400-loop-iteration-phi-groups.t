# A decoded byte traverses eight guarded steps. The root stays live while
# successive node/count phis need registers of their own.
function @touch(%p : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    return ptr %p
}

function @walk(%root : ptr, %input : ptr, %length : i64, %budget : i32, %output : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %r = cmp ne ptr %root, 0
    %b = cmp gt i32 %budget, 0
    %ok = binary and i64 %r, %b
    branch %ok, ^start, ^empty
  block ^start:
    %storage = call ptr @touch(%output)
    jump ^head
  block ^head:
    %index = phi i64 [^start: 0, ^tail: %index_next]
    %node = phi ptr [^start: %root, ^tail: %last_node]
    %count = phi i64 [^start: 0, ^tail: %last_count]
    %done = phi i32 [^start: 0, ^tail: %last_done]
    %within = cmp ult i64 %index, %length
    branch %within, ^byte, ^finish
  block ^byte:
    %at = index u8 %input, %index
    %byte_value = load u8 %at
    %wide = convert zext i64 u8 %byte_value
    jump ^step0
  block ^step0:
    %done0 = binary add i32 %done, 1
    %shift0 = binary ushr i64 %wide, 7
    %bit0 = binary and i64 %shift0, 1
    %scaled0 = binary mul i64 %bit0, 8
    %offset0 = binary add i64 %scaled0, 8
    %child_at0 = index i8 %node, %offset0
    %child0 = load ptr %child_at0
    %null0 = cmp eq ptr %child0, 0
    branch %null0, ^tail, ^check0
  block ^check0:
    %flag_at0 = index i8 %child0, 5
    %flag0 = load u8 %flag_at0
    branch %flag0, ^leaf0, ^merge0
  block ^leaf0:
    %value_at0 = index i8 %child0, 4
    %value0 = load u8 %value_at0
    %dest0 = index u8 %storage, %count
    store u8 %value0, %dest0
    %advanced0 = binary add i64 %count, 1
    jump ^merge0
  block ^merge0:
    %node0 = phi ptr [^check0: %child0, ^leaf0: %root]
    %count0 = phi i64 [^check0: %count, ^leaf0: %advanced0]
    %more0 = cmp lt i32 %done0, %budget
    branch %more0, ^step1, ^tail
  block ^step1:
    %done1 = binary add i32 %done, 2
    %shift1 = binary ushr i64 %wide, 6
    %bit1 = binary and i64 %shift1, 1
    %scaled1 = binary mul i64 %bit1, 8
    %offset1 = binary add i64 %scaled1, 8
    %child_at1 = index i8 %node0, %offset1
    %child1 = load ptr %child_at1
    %null1 = cmp eq ptr %child1, 0
    branch %null1, ^tail, ^check1
  block ^check1:
    %flag_at1 = index i8 %child1, 5
    %flag1 = load u8 %flag_at1
    branch %flag1, ^leaf1, ^merge1
  block ^leaf1:
    %value_at1 = index i8 %child1, 4
    %value1 = load u8 %value_at1
    %dest1 = index u8 %storage, %count0
    store u8 %value1, %dest1
    %advanced1 = binary add i64 %count0, 1
    jump ^merge1
  block ^merge1:
    %node1 = phi ptr [^check1: %child1, ^leaf1: %root]
    %count1 = phi i64 [^check1: %count0, ^leaf1: %advanced1]
    %more1 = cmp lt i32 %done1, %budget
    branch %more1, ^step2, ^tail
  block ^step2:
    %done2 = binary add i32 %done, 3
    %shift2 = binary ushr i64 %wide, 5
    %bit2 = binary and i64 %shift2, 1
    %scaled2 = binary mul i64 %bit2, 8
    %offset2 = binary add i64 %scaled2, 8
    %child_at2 = index i8 %node1, %offset2
    %child2 = load ptr %child_at2
    %null2 = cmp eq ptr %child2, 0
    branch %null2, ^tail, ^check2
  block ^check2:
    %flag_at2 = index i8 %child2, 5
    %flag2 = load u8 %flag_at2
    branch %flag2, ^leaf2, ^merge2
  block ^leaf2:
    %value_at2 = index i8 %child2, 4
    %value2 = load u8 %value_at2
    %dest2 = index u8 %storage, %count1
    store u8 %value2, %dest2
    %advanced2 = binary add i64 %count1, 1
    jump ^merge2
  block ^merge2:
    %node2 = phi ptr [^check2: %child2, ^leaf2: %root]
    %count2 = phi i64 [^check2: %count1, ^leaf2: %advanced2]
    %more2 = cmp lt i32 %done2, %budget
    branch %more2, ^step3, ^tail
  block ^step3:
    %done3 = binary add i32 %done, 4
    %shift3 = binary ushr i64 %wide, 4
    %bit3 = binary and i64 %shift3, 1
    %scaled3 = binary mul i64 %bit3, 8
    %offset3 = binary add i64 %scaled3, 8
    %child_at3 = index i8 %node2, %offset3
    %child3 = load ptr %child_at3
    %null3 = cmp eq ptr %child3, 0
    branch %null3, ^tail, ^check3
  block ^check3:
    %flag_at3 = index i8 %child3, 5
    %flag3 = load u8 %flag_at3
    branch %flag3, ^leaf3, ^merge3
  block ^leaf3:
    %value_at3 = index i8 %child3, 4
    %value3 = load u8 %value_at3
    %dest3 = index u8 %storage, %count2
    store u8 %value3, %dest3
    %advanced3 = binary add i64 %count2, 1
    jump ^merge3
  block ^merge3:
    %node3 = phi ptr [^check3: %child3, ^leaf3: %root]
    %count3 = phi i64 [^check3: %count2, ^leaf3: %advanced3]
    %more3 = cmp lt i32 %done3, %budget
    branch %more3, ^step4, ^tail
  block ^step4:
    %done4 = binary add i32 %done, 5
    %shift4 = binary ushr i64 %wide, 3
    %bit4 = binary and i64 %shift4, 1
    %scaled4 = binary mul i64 %bit4, 8
    %offset4 = binary add i64 %scaled4, 8
    %child_at4 = index i8 %node3, %offset4
    %child4 = load ptr %child_at4
    %null4 = cmp eq ptr %child4, 0
    branch %null4, ^tail, ^check4
  block ^check4:
    %flag_at4 = index i8 %child4, 5
    %flag4 = load u8 %flag_at4
    branch %flag4, ^leaf4, ^merge4
  block ^leaf4:
    %value_at4 = index i8 %child4, 4
    %value4 = load u8 %value_at4
    %dest4 = index u8 %storage, %count3
    store u8 %value4, %dest4
    %advanced4 = binary add i64 %count3, 1
    jump ^merge4
  block ^merge4:
    %node4 = phi ptr [^check4: %child4, ^leaf4: %root]
    %count4 = phi i64 [^check4: %count3, ^leaf4: %advanced4]
    %more4 = cmp lt i32 %done4, %budget
    branch %more4, ^step5, ^tail
  block ^step5:
    %done5 = binary add i32 %done, 6
    %shift5 = binary ushr i64 %wide, 2
    %bit5 = binary and i64 %shift5, 1
    %scaled5 = binary mul i64 %bit5, 8
    %offset5 = binary add i64 %scaled5, 8
    %child_at5 = index i8 %node4, %offset5
    %child5 = load ptr %child_at5
    %null5 = cmp eq ptr %child5, 0
    branch %null5, ^tail, ^check5
  block ^check5:
    %flag_at5 = index i8 %child5, 5
    %flag5 = load u8 %flag_at5
    branch %flag5, ^leaf5, ^merge5
  block ^leaf5:
    %value_at5 = index i8 %child5, 4
    %value5 = load u8 %value_at5
    %dest5 = index u8 %storage, %count4
    store u8 %value5, %dest5
    %advanced5 = binary add i64 %count4, 1
    jump ^merge5
  block ^merge5:
    %node5 = phi ptr [^check5: %child5, ^leaf5: %root]
    %count5 = phi i64 [^check5: %count4, ^leaf5: %advanced5]
    %more5 = cmp lt i32 %done5, %budget
    branch %more5, ^step6, ^tail
  block ^step6:
    %done6 = binary add i32 %done, 7
    %shift6 = binary ushr i64 %wide, 1
    %bit6 = binary and i64 %shift6, 1
    %scaled6 = binary mul i64 %bit6, 8
    %offset6 = binary add i64 %scaled6, 8
    %child_at6 = index i8 %node5, %offset6
    %child6 = load ptr %child_at6
    %null6 = cmp eq ptr %child6, 0
    branch %null6, ^tail, ^check6
  block ^check6:
    %flag_at6 = index i8 %child6, 5
    %flag6 = load u8 %flag_at6
    branch %flag6, ^leaf6, ^merge6
  block ^leaf6:
    %value_at6 = index i8 %child6, 4
    %value6 = load u8 %value_at6
    %dest6 = index u8 %storage, %count5
    store u8 %value6, %dest6
    %advanced6 = binary add i64 %count5, 1
    jump ^merge6
  block ^merge6:
    %node6 = phi ptr [^check6: %child6, ^leaf6: %root]
    %count6 = phi i64 [^check6: %count5, ^leaf6: %advanced6]
    %more6 = cmp lt i32 %done6, %budget
    branch %more6, ^step7, ^tail
  block ^step7:
    %done7 = binary add i32 %done, 8
    %shift7 = binary ushr i64 %wide, 0
    %bit7 = binary and i64 %shift7, 1
    %scaled7 = binary mul i64 %bit7, 8
    %offset7 = binary add i64 %scaled7, 8
    %child_at7 = index i8 %node6, %offset7
    %child7 = load ptr %child_at7
    %null7 = cmp eq ptr %child7, 0
    branch %null7, ^tail, ^check7
  block ^check7:
    %flag_at7 = index i8 %child7, 5
    %flag7 = load u8 %flag_at7
    branch %flag7, ^leaf7, ^merge7
  block ^leaf7:
    %value_at7 = index i8 %child7, 4
    %value7 = load u8 %value_at7
    %dest7 = index u8 %storage, %count6
    store u8 %value7, %dest7
    %advanced7 = binary add i64 %count6, 1
    jump ^merge7
  block ^merge7:
    %node7 = phi ptr [^check7: %child7, ^leaf7: %root]
    %count7 = phi i64 [^check7: %count6, ^leaf7: %advanced7]
    jump ^tail
  block ^tail:
    %last_done = phi i32 [^step0: %done0, ^merge0: %done0, ^step1: %done1, ^merge1: %done1, ^step2: %done2, ^merge2: %done2, ^step3: %done3, ^merge3: %done3, ^step4: %done4, ^merge4: %done4, ^step5: %done5, ^merge5: %done5, ^step6: %done6, ^merge6: %done6, ^step7: %done7, ^merge7: %done7]
    %last_count = phi i64 [^step0: %count, ^merge0: %count0, ^step1: %count0, ^merge1: %count1, ^step2: %count1, ^merge2: %count2, ^step3: %count2, ^merge3: %count3, ^step4: %count3, ^merge4: %count4, ^step5: %count4, ^merge5: %count5, ^step6: %count5, ^merge6: %count6, ^step7: %count6, ^merge7: %count7]
    %last_node = phi ptr [^step0: %child0, ^merge0: %node0, ^step1: %child1, ^merge1: %node1, ^step2: %child2, ^merge2: %node2, ^step3: %child3, ^merge3: %node3, ^step4: %child4, ^merge4: %node4, ^step5: %child5, ^merge5: %node5, ^step6: %child6, ^merge6: %node6, ^step7: %child7, ^merge7: %node7]
    %index_next = binary add i64 %index, 1
    %again = cmp lt i32 %last_done, %budget
    branch %again, ^head, ^finish_tail
  block ^finish:
    return i64 %count
  block ^finish_tail:
    return i64 %last_count
  block ^empty:
    return i64 0
}

function @main() -> i32 [role=entry] {
  slot $root : obj<24x8>
  slot $left : obj<24x8>
  slot $right : obj<24x8>
  slot $bytes : obj<2x1>
  slot $out : obj<16x1>
  block ^entry:
    %root = addr $root
    %left = addr $left
    %right = addr $right
    %bytes = addr $bytes
    %out = addr $out
    %lvalue = index i8 %left, 4
    %rvalue = index i8 %right, 4
    %lflag = index i8 %left, 5
    %rflag = index i8 %right, 5
    store u8 2, %lvalue
    store u8 7, %rvalue
    store u8 1, %lflag
    store u8 1, %rflag
    %lp = index i8 %root, 8
    %rp = index i8 %root, 16
    store ptr %left, %lp
    store ptr %right, %rp
    store u8 170, %bytes
    %second = index i8 %bytes, 1
    store u8 240, %second
    %count = call i64 @walk(%root, %bytes, 2, 13, %out)
    %bad_count = cmp ne i64 %count, 13
    jump ^sum
  block ^sum:
    %i = phi i64 [^entry: 0, ^sum: %next]
    %total = phi i64 [^entry: 0, ^sum: %added]
    %p = index u8 %out, %i
    %v = load u8 %p
    %wide = convert zext i64 u8 %v
    %added = binary add i64 %total, %wide
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, 13
    branch %more, ^sum, ^finish
  block ^finish:
    %bad_sum = cmp ne i64 %added, 66
    %empty = call i64 @walk(%root, %bytes, 2, 0, %out)
    %bad_empty = cmp ne i64 %empty, 0
    store ptr 0, %rp
    %missing = call i64 @walk(%root, %bytes, 1, 8, %out)
    %bad_missing = cmp ne i64 %missing, 0
    %bad0 = binary or i64 %bad_count, %bad_sum
    %bad1 = binary or i64 %bad0, %bad_empty
    %bad2 = binary or i64 %bad1, %bad_missing
    %result = convert trunc i32 i64 %bad2
    return i32 %result
}
