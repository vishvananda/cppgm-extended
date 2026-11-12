# Multiple entries to one cycle require general control-flow lowering.
target wasm32 1

function @dispatch_cycle(%start : i32, %limit : i32) -> i32 {
  block ^entry:
    switch %start, ^b0, 1:^b1, 2:^b2, 3:^b3, 4:^b4, 5:^b5, 6:^b6, 7:^b7, 8:^b8, 9:^b9, 10:^b10, 11:^b11, 12:^b12, 13:^b13, 14:^b14, 15:^b15
  block ^b0:
    %count0 = phi i32 [^entry: 0, ^b15: %next15]
    %next0 = binary add i32 %count0, 1
    %done0 = cmp ge i32 %next0, %limit
    branch %done0, ^exit, ^b1
  block ^b1:
    %count1 = phi i32 [^entry: 0, ^b0: %next0]
    %next1 = binary add i32 %count1, 1
    %done1 = cmp ge i32 %next1, %limit
    branch %done1, ^exit, ^b2
  block ^b2:
    %count2 = phi i32 [^entry: 0, ^b1: %next1]
    %next2 = binary add i32 %count2, 1
    %done2 = cmp ge i32 %next2, %limit
    branch %done2, ^exit, ^b3
  block ^b3:
    %count3 = phi i32 [^entry: 0, ^b2: %next2]
    %next3 = binary add i32 %count3, 1
    %done3 = cmp ge i32 %next3, %limit
    branch %done3, ^exit, ^b4
  block ^b4:
    %count4 = phi i32 [^entry: 0, ^b3: %next3]
    %next4 = binary add i32 %count4, 1
    %done4 = cmp ge i32 %next4, %limit
    branch %done4, ^exit, ^b5
  block ^b5:
    %count5 = phi i32 [^entry: 0, ^b4: %next4]
    %next5 = binary add i32 %count5, 1
    %done5 = cmp ge i32 %next5, %limit
    branch %done5, ^exit, ^b6
  block ^b6:
    %count6 = phi i32 [^entry: 0, ^b5: %next5]
    %next6 = binary add i32 %count6, 1
    %done6 = cmp ge i32 %next6, %limit
    branch %done6, ^exit, ^b7
  block ^b7:
    %count7 = phi i32 [^entry: 0, ^b6: %next6]
    %next7 = binary add i32 %count7, 1
    %done7 = cmp ge i32 %next7, %limit
    branch %done7, ^exit, ^b8
  block ^b8:
    %count8 = phi i32 [^entry: 0, ^b7: %next7]
    %next8 = binary add i32 %count8, 1
    %done8 = cmp ge i32 %next8, %limit
    branch %done8, ^exit, ^b9
  block ^b9:
    %count9 = phi i32 [^entry: 0, ^b8: %next8]
    %next9 = binary add i32 %count9, 1
    %done9 = cmp ge i32 %next9, %limit
    branch %done9, ^exit, ^b10
  block ^b10:
    %count10 = phi i32 [^entry: 0, ^b9: %next9]
    %next10 = binary add i32 %count10, 1
    %done10 = cmp ge i32 %next10, %limit
    branch %done10, ^exit, ^b11
  block ^b11:
    %count11 = phi i32 [^entry: 0, ^b10: %next10]
    %next11 = binary add i32 %count11, 1
    %done11 = cmp ge i32 %next11, %limit
    branch %done11, ^exit, ^b12
  block ^b12:
    %count12 = phi i32 [^entry: 0, ^b11: %next11]
    %next12 = binary add i32 %count12, 1
    %done12 = cmp ge i32 %next12, %limit
    branch %done12, ^exit, ^b13
  block ^b13:
    %count13 = phi i32 [^entry: 0, ^b12: %next12]
    %next13 = binary add i32 %count13, 1
    %done13 = cmp ge i32 %next13, %limit
    branch %done13, ^exit, ^b14
  block ^b14:
    %count14 = phi i32 [^entry: 0, ^b13: %next13]
    %next14 = binary add i32 %count14, 1
    %done14 = cmp ge i32 %next14, %limit
    branch %done14, ^exit, ^b15
  block ^b15:
    %count15 = phi i32 [^entry: 0, ^b14: %next14]
    %next15 = binary add i32 %count15, 1
    %done15 = cmp ge i32 %next15, %limit
    branch %done15, ^exit, ^b0
  block ^exit:
    %result = phi i32 [^b0: %next0, ^b1: %next1, ^b2: %next2, ^b3: %next3, ^b4: %next4, ^b5: %next5, ^b6: %next6, ^b7: %next7, ^b8: %next8, ^b9: %next9, ^b10: %next10, ^b11: %next11, ^b12: %next12, ^b13: %next13, ^b14: %next14, ^b15: %next15]
    return i32 %result
}
