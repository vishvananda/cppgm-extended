function @signed(%which : i32, %keep : i64) -> i64 [unwind=no] {
  block ^entry:
    switch %which, ^fallback, -4:^c0, -3:^c1, -2:^c2, -1:^c3, 0:^c4, 1:^c5, 2:^c6, 3:^c7, 4:^c8, 5:^c9, 6:^c10, 7:^c11
  block ^c0:
    %result0 = binary add i64 %keep, 7
    return i64 %result0
  block ^c1:
    %result1 = binary add i64 %keep, 20
    return i64 %result1
  block ^c2:
    %result2 = binary add i64 %keep, 33
    return i64 %result2
  block ^c3:
    %result3 = binary add i64 %keep, 46
    return i64 %result3
  block ^c4:
    %result4 = binary add i64 %keep, 59
    return i64 %result4
  block ^c5:
    %result5 = binary add i64 %keep, 72
    return i64 %result5
  block ^c6:
    %result6 = binary add i64 %keep, 85
    return i64 %result6
  block ^c7:
    %result7 = binary add i64 %keep, 98
    return i64 %result7
  block ^c8:
    %result8 = binary add i64 %keep, 111
    return i64 %result8
  block ^c9:
    %result9 = binary add i64 %keep, 124
    return i64 %result9
  block ^c10:
    %result10 = binary add i64 %keep, 137
    return i64 %result10
  block ^c11:
    %result11 = binary add i64 %keep, 150
    return i64 %result11
  block ^fallback:
    return i64 -999
}
function @unsigned(%which : u32, %keep : i64) -> i64 [unwind=no] {
  block ^entry:
    switch %which, ^fallback, 2147483644:^c0, 2147483645:^c1, 2147483646:^c2, 2147483647:^c3, 2147483648:^c4, 2147483649:^c5, 2147483650:^c6, 2147483651:^c7, 2147483652:^c8, 2147483653:^c9, 2147483654:^c10, 2147483655:^c11
  block ^c0:
    %result0 = binary add i64 %keep, 7
    return i64 %result0
  block ^c1:
    %result1 = binary add i64 %keep, 20
    return i64 %result1
  block ^c2:
    %result2 = binary add i64 %keep, 33
    return i64 %result2
  block ^c3:
    %result3 = binary add i64 %keep, 46
    return i64 %result3
  block ^c4:
    %result4 = binary add i64 %keep, 59
    return i64 %result4
  block ^c5:
    %result5 = binary add i64 %keep, 72
    return i64 %result5
  block ^c6:
    %result6 = binary add i64 %keep, 85
    return i64 %result6
  block ^c7:
    %result7 = binary add i64 %keep, 98
    return i64 %result7
  block ^c8:
    %result8 = binary add i64 %keep, 111
    return i64 %result8
  block ^c9:
    %result9 = binary add i64 %keep, 124
    return i64 %result9
  block ^c10:
    %result10 = binary add i64 %keep, 137
    return i64 %result10
  block ^c11:
    %result11 = binary add i64 %keep, 150
    return i64 %result11
  block ^fallback:
    return i64 -999
}
function @holes(%which : i64, %keep : i64) -> i64 [unwind=no] {
  block ^entry:
    switch %which, ^fallback, 0:^c0, 2:^c1, 4:^c2, 6:^c3, 8:^c4, 10:^c5, 12:^c6, 14:^c7
  block ^c0:
    %result0 = binary add i64 %keep, 7
    return i64 %result0
  block ^c1:
    %result1 = binary add i64 %keep, 20
    return i64 %result1
  block ^c2:
    %result2 = binary add i64 %keep, 33
    return i64 %result2
  block ^c3:
    %result3 = binary add i64 %keep, 46
    return i64 %result3
  block ^c4:
    %result4 = binary add i64 %keep, 59
    return i64 %result4
  block ^c5:
    %result5 = binary add i64 %keep, 72
    return i64 %result5
  block ^c6:
    %result6 = binary add i64 %keep, 85
    return i64 %result6
  block ^c7:
    %result7 = binary add i64 %keep, 98
    return i64 %result7
  block ^fallback:
    return i64 -999
}
function @negative(%which : i64, %keep : i64) -> i64 [unwind=no] {
  block ^entry:
    switch %which, ^fallback, -20:^c0, -19:^c1, -18:^c2, -17:^c3, -16:^c4, -15:^c5, -14:^c6, -13:^c7, -12:^c8, -11:^c9, -10:^c10, -9:^c11
  block ^c0:
    %result0 = binary add i64 %keep, 7
    return i64 %result0
  block ^c1:
    %result1 = binary add i64 %keep, 20
    return i64 %result1
  block ^c2:
    %result2 = binary add i64 %keep, 33
    return i64 %result2
  block ^c3:
    %result3 = binary add i64 %keep, 46
    return i64 %result3
  block ^c4:
    %result4 = binary add i64 %keep, 59
    return i64 %result4
  block ^c5:
    %result5 = binary add i64 %keep, 72
    return i64 %result5
  block ^c6:
    %result6 = binary add i64 %keep, 85
    return i64 %result6
  block ^c7:
    %result7 = binary add i64 %keep, 98
    return i64 %result7
  block ^c8:
    %result8 = binary add i64 %keep, 111
    return i64 %result8
  block ^c9:
    %result9 = binary add i64 %keep, 124
    return i64 %result9
  block ^c10:
    %result10 = binary add i64 %keep, 137
    return i64 %result10
  block ^c11:
    %result11 = binary add i64 %keep, 150
    return i64 %result11
  block ^fallback:
    return i64 -999
}
function @duplicate(%which : i64, %keep : i64) -> i64 [unwind=no] {
  block ^entry:
    switch %which, ^fallback, 1:^c0, 2:^c1, 3:^c2, 4:^c3, 5:^c4, 6:^c5, 7:^c6, 8:^c7, 1:^c8
  block ^c0:
    %result0 = binary add i64 %keep, 7
    return i64 %result0
  block ^c1:
    %result1 = binary add i64 %keep, 20
    return i64 %result1
  block ^c2:
    %result2 = binary add i64 %keep, 33
    return i64 %result2
  block ^c3:
    %result3 = binary add i64 %keep, 46
    return i64 %result3
  block ^c4:
    %result4 = binary add i64 %keep, 59
    return i64 %result4
  block ^c5:
    %result5 = binary add i64 %keep, 72
    return i64 %result5
  block ^c6:
    %result6 = binary add i64 %keep, 85
    return i64 %result6
  block ^c7:
    %result7 = binary add i64 %keep, 98
    return i64 %result7
  block ^c8:
    %result8 = binary add i64 %keep, 111
    return i64 %result8
  block ^fallback:
    return i64 -999
}
function @sparse(%which : i64, %keep : i64) -> i64 [unwind=no] {
  block ^entry:
    switch %which, ^fallback, 0:^c0, 10000:^c1, 20000:^c2, 30000:^c3, 40000:^c4, 50000:^c5
  block ^c0:
    %result0 = binary add i64 %keep, 7
    return i64 %result0
  block ^c1:
    %result1 = binary add i64 %keep, 20
    return i64 %result1
  block ^c2:
    %result2 = binary add i64 %keep, 33
    return i64 %result2
  block ^c3:
    %result3 = binary add i64 %keep, 46
    return i64 %result3
  block ^c4:
    %result4 = binary add i64 %keep, 59
    return i64 %result4
  block ^c5:
    %result5 = binary add i64 %keep, 72
    return i64 %result5
  block ^fallback:
    return i64 -999
}
function @main() -> i32 [role=entry, unwind=no] {
  block ^entry:
    %result0 = call i64 @signed(-6, 53)
    %bad0 = cmp ne i64 %result0, -999
    %result1 = call i64 @signed(-5, 53)
    %bad1 = cmp ne i64 %result1, -999
    %any1 = binary or i1 %bad0, %bad1
    %result2 = call i64 @signed(-4, 53)
    %bad2 = cmp ne i64 %result2, 60
    %any2 = binary or i1 %any1, %bad2
    %result3 = call i64 @signed(-3, 53)
    %bad3 = cmp ne i64 %result3, 73
    %any3 = binary or i1 %any2, %bad3
    %result4 = call i64 @signed(-2, 53)
    %bad4 = cmp ne i64 %result4, 86
    %any4 = binary or i1 %any3, %bad4
    %result5 = call i64 @signed(-1, 53)
    %bad5 = cmp ne i64 %result5, 99
    %any5 = binary or i1 %any4, %bad5
    %result6 = call i64 @signed(0, 53)
    %bad6 = cmp ne i64 %result6, 112
    %any6 = binary or i1 %any5, %bad6
    %result7 = call i64 @signed(1, 53)
    %bad7 = cmp ne i64 %result7, 125
    %any7 = binary or i1 %any6, %bad7
    %result8 = call i64 @signed(2, 53)
    %bad8 = cmp ne i64 %result8, 138
    %any8 = binary or i1 %any7, %bad8
    %result9 = call i64 @signed(3, 53)
    %bad9 = cmp ne i64 %result9, 151
    %any9 = binary or i1 %any8, %bad9
    %result10 = call i64 @signed(4, 53)
    %bad10 = cmp ne i64 %result10, 164
    %any10 = binary or i1 %any9, %bad10
    %result11 = call i64 @signed(5, 53)
    %bad11 = cmp ne i64 %result11, 177
    %any11 = binary or i1 %any10, %bad11
    %result12 = call i64 @signed(6, 53)
    %bad12 = cmp ne i64 %result12, 190
    %any12 = binary or i1 %any11, %bad12
    %result13 = call i64 @signed(7, 53)
    %bad13 = cmp ne i64 %result13, 203
    %any13 = binary or i1 %any12, %bad13
    %result14 = call i64 @signed(8, 53)
    %bad14 = cmp ne i64 %result14, -999
    %any14 = binary or i1 %any13, %bad14
    %result15 = call i64 @signed(9, 53)
    %bad15 = cmp ne i64 %result15, -999
    %any15 = binary or i1 %any14, %bad15
    %result16 = call i64 @unsigned(2147483642, 53)
    %bad16 = cmp ne i64 %result16, -999
    %any16 = binary or i1 %any15, %bad16
    %result17 = call i64 @unsigned(2147483643, 53)
    %bad17 = cmp ne i64 %result17, -999
    %any17 = binary or i1 %any16, %bad17
    %result18 = call i64 @unsigned(2147483644, 53)
    %bad18 = cmp ne i64 %result18, 60
    %any18 = binary or i1 %any17, %bad18
    %result19 = call i64 @unsigned(2147483645, 53)
    %bad19 = cmp ne i64 %result19, 73
    %any19 = binary or i1 %any18, %bad19
    %result20 = call i64 @unsigned(2147483646, 53)
    %bad20 = cmp ne i64 %result20, 86
    %any20 = binary or i1 %any19, %bad20
    %result21 = call i64 @unsigned(2147483647, 53)
    %bad21 = cmp ne i64 %result21, 99
    %any21 = binary or i1 %any20, %bad21
    %result22 = call i64 @unsigned(2147483648, 53)
    %bad22 = cmp ne i64 %result22, 112
    %any22 = binary or i1 %any21, %bad22
    %result23 = call i64 @unsigned(2147483649, 53)
    %bad23 = cmp ne i64 %result23, 125
    %any23 = binary or i1 %any22, %bad23
    %result24 = call i64 @unsigned(2147483650, 53)
    %bad24 = cmp ne i64 %result24, 138
    %any24 = binary or i1 %any23, %bad24
    %result25 = call i64 @unsigned(2147483651, 53)
    %bad25 = cmp ne i64 %result25, 151
    %any25 = binary or i1 %any24, %bad25
    %result26 = call i64 @unsigned(2147483652, 53)
    %bad26 = cmp ne i64 %result26, 164
    %any26 = binary or i1 %any25, %bad26
    %result27 = call i64 @unsigned(2147483653, 53)
    %bad27 = cmp ne i64 %result27, 177
    %any27 = binary or i1 %any26, %bad27
    %result28 = call i64 @unsigned(2147483654, 53)
    %bad28 = cmp ne i64 %result28, 190
    %any28 = binary or i1 %any27, %bad28
    %result29 = call i64 @unsigned(2147483655, 53)
    %bad29 = cmp ne i64 %result29, 203
    %any29 = binary or i1 %any28, %bad29
    %result30 = call i64 @unsigned(2147483656, 53)
    %bad30 = cmp ne i64 %result30, -999
    %any30 = binary or i1 %any29, %bad30
    %result31 = call i64 @unsigned(2147483657, 53)
    %bad31 = cmp ne i64 %result31, -999
    %any31 = binary or i1 %any30, %bad31
    %result32 = call i64 @holes(-2, 53)
    %bad32 = cmp ne i64 %result32, -999
    %any32 = binary or i1 %any31, %bad32
    %result33 = call i64 @holes(-1, 53)
    %bad33 = cmp ne i64 %result33, -999
    %any33 = binary or i1 %any32, %bad33
    %result34 = call i64 @holes(0, 53)
    %bad34 = cmp ne i64 %result34, 60
    %any34 = binary or i1 %any33, %bad34
    %result35 = call i64 @holes(1, 53)
    %bad35 = cmp ne i64 %result35, -999
    %any35 = binary or i1 %any34, %bad35
    %result36 = call i64 @holes(2, 53)
    %bad36 = cmp ne i64 %result36, 73
    %any36 = binary or i1 %any35, %bad36
    %result37 = call i64 @holes(3, 53)
    %bad37 = cmp ne i64 %result37, -999
    %any37 = binary or i1 %any36, %bad37
    %result38 = call i64 @holes(4, 53)
    %bad38 = cmp ne i64 %result38, 86
    %any38 = binary or i1 %any37, %bad38
    %result39 = call i64 @holes(5, 53)
    %bad39 = cmp ne i64 %result39, -999
    %any39 = binary or i1 %any38, %bad39
    %result40 = call i64 @holes(6, 53)
    %bad40 = cmp ne i64 %result40, 99
    %any40 = binary or i1 %any39, %bad40
    %result41 = call i64 @holes(7, 53)
    %bad41 = cmp ne i64 %result41, -999
    %any41 = binary or i1 %any40, %bad41
    %result42 = call i64 @holes(8, 53)
    %bad42 = cmp ne i64 %result42, 112
    %any42 = binary or i1 %any41, %bad42
    %result43 = call i64 @holes(9, 53)
    %bad43 = cmp ne i64 %result43, -999
    %any43 = binary or i1 %any42, %bad43
    %result44 = call i64 @holes(10, 53)
    %bad44 = cmp ne i64 %result44, 125
    %any44 = binary or i1 %any43, %bad44
    %result45 = call i64 @holes(11, 53)
    %bad45 = cmp ne i64 %result45, -999
    %any45 = binary or i1 %any44, %bad45
    %result46 = call i64 @holes(12, 53)
    %bad46 = cmp ne i64 %result46, 138
    %any46 = binary or i1 %any45, %bad46
    %result47 = call i64 @holes(13, 53)
    %bad47 = cmp ne i64 %result47, -999
    %any47 = binary or i1 %any46, %bad47
    %result48 = call i64 @holes(14, 53)
    %bad48 = cmp ne i64 %result48, 151
    %any48 = binary or i1 %any47, %bad48
    %result49 = call i64 @holes(15, 53)
    %bad49 = cmp ne i64 %result49, -999
    %any49 = binary or i1 %any48, %bad49
    %result50 = call i64 @holes(16, 53)
    %bad50 = cmp ne i64 %result50, -999
    %any50 = binary or i1 %any49, %bad50
    %result51 = call i64 @negative(-22, 53)
    %bad51 = cmp ne i64 %result51, -999
    %any51 = binary or i1 %any50, %bad51
    %result52 = call i64 @negative(-21, 53)
    %bad52 = cmp ne i64 %result52, -999
    %any52 = binary or i1 %any51, %bad52
    %result53 = call i64 @negative(-20, 53)
    %bad53 = cmp ne i64 %result53, 60
    %any53 = binary or i1 %any52, %bad53
    %result54 = call i64 @negative(-19, 53)
    %bad54 = cmp ne i64 %result54, 73
    %any54 = binary or i1 %any53, %bad54
    %result55 = call i64 @negative(-18, 53)
    %bad55 = cmp ne i64 %result55, 86
    %any55 = binary or i1 %any54, %bad55
    %result56 = call i64 @negative(-17, 53)
    %bad56 = cmp ne i64 %result56, 99
    %any56 = binary or i1 %any55, %bad56
    %result57 = call i64 @negative(-16, 53)
    %bad57 = cmp ne i64 %result57, 112
    %any57 = binary or i1 %any56, %bad57
    %result58 = call i64 @negative(-15, 53)
    %bad58 = cmp ne i64 %result58, 125
    %any58 = binary or i1 %any57, %bad58
    %result59 = call i64 @negative(-14, 53)
    %bad59 = cmp ne i64 %result59, 138
    %any59 = binary or i1 %any58, %bad59
    %result60 = call i64 @negative(-13, 53)
    %bad60 = cmp ne i64 %result60, 151
    %any60 = binary or i1 %any59, %bad60
    %result61 = call i64 @negative(-12, 53)
    %bad61 = cmp ne i64 %result61, 164
    %any61 = binary or i1 %any60, %bad61
    %result62 = call i64 @negative(-11, 53)
    %bad62 = cmp ne i64 %result62, 177
    %any62 = binary or i1 %any61, %bad62
    %result63 = call i64 @negative(-10, 53)
    %bad63 = cmp ne i64 %result63, 190
    %any63 = binary or i1 %any62, %bad63
    %result64 = call i64 @negative(-9, 53)
    %bad64 = cmp ne i64 %result64, 203
    %any64 = binary or i1 %any63, %bad64
    %result65 = call i64 @negative(-8, 53)
    %bad65 = cmp ne i64 %result65, -999
    %any65 = binary or i1 %any64, %bad65
    %result66 = call i64 @negative(-7, 53)
    %bad66 = cmp ne i64 %result66, -999
    %any66 = binary or i1 %any65, %bad66
    %result67 = call i64 @duplicate(-1, 53)
    %bad67 = cmp ne i64 %result67, -999
    %any67 = binary or i1 %any66, %bad67
    %result68 = call i64 @duplicate(0, 53)
    %bad68 = cmp ne i64 %result68, -999
    %any68 = binary or i1 %any67, %bad68
    %result69 = call i64 @duplicate(1, 53)
    %bad69 = cmp ne i64 %result69, 60
    %any69 = binary or i1 %any68, %bad69
    %result70 = call i64 @duplicate(2, 53)
    %bad70 = cmp ne i64 %result70, 73
    %any70 = binary or i1 %any69, %bad70
    %result71 = call i64 @duplicate(3, 53)
    %bad71 = cmp ne i64 %result71, 86
    %any71 = binary or i1 %any70, %bad71
    %result72 = call i64 @duplicate(4, 53)
    %bad72 = cmp ne i64 %result72, 99
    %any72 = binary or i1 %any71, %bad72
    %result73 = call i64 @duplicate(5, 53)
    %bad73 = cmp ne i64 %result73, 112
    %any73 = binary or i1 %any72, %bad73
    %result74 = call i64 @duplicate(6, 53)
    %bad74 = cmp ne i64 %result74, 125
    %any74 = binary or i1 %any73, %bad74
    %result75 = call i64 @duplicate(7, 53)
    %bad75 = cmp ne i64 %result75, 138
    %any75 = binary or i1 %any74, %bad75
    %result76 = call i64 @duplicate(8, 53)
    %bad76 = cmp ne i64 %result76, 151
    %any76 = binary or i1 %any75, %bad76
    %result77 = call i64 @duplicate(9, 53)
    %bad77 = cmp ne i64 %result77, -999
    %any77 = binary or i1 %any76, %bad77
    %result78 = call i64 @duplicate(10, 53)
    %bad78 = cmp ne i64 %result78, -999
    %any78 = binary or i1 %any77, %bad78
    %result79 = call i64 @sparse(-1, 53)
    %bad79 = cmp ne i64 %result79, -999
    %any79 = binary or i1 %any78, %bad79
    %result80 = call i64 @sparse(0, 53)
    %bad80 = cmp ne i64 %result80, 60
    %any80 = binary or i1 %any79, %bad80
    %result81 = call i64 @sparse(10000, 53)
    %bad81 = cmp ne i64 %result81, 73
    %any81 = binary or i1 %any80, %bad81
    %result82 = call i64 @sparse(20000, 53)
    %bad82 = cmp ne i64 %result82, 86
    %any82 = binary or i1 %any81, %bad82
    %result83 = call i64 @sparse(30000, 53)
    %bad83 = cmp ne i64 %result83, 99
    %any83 = binary or i1 %any82, %bad83
    %result84 = call i64 @sparse(40000, 53)
    %bad84 = cmp ne i64 %result84, 112
    %any84 = binary or i1 %any83, %bad84
    %result85 = call i64 @sparse(50000, 53)
    %bad85 = cmp ne i64 %result85, 125
    %any85 = binary or i1 %any84, %bad85
    %result86 = call i64 @sparse(50001, 53)
    %bad86 = cmp ne i64 %result86, -999
    %any86 = binary or i1 %any85, %bad86
    %result87 = call i64 @sparse(2147483647, 53)
    %bad87 = cmp ne i64 %result87, -999
    %any87 = binary or i1 %any86, %bad87
    %result88 = call i64 @sparse(-2147483648, 53)
    %bad88 = cmp ne i64 %result88, -999
    %any88 = binary or i1 %any87, %bad88
    %exit = convert zext i32 i1 %any88
    return i32 %exit
}
