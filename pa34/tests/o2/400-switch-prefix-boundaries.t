function @signed_prefix(%value : i32) -> i64 [binding=strong] {
  block ^entry:
    switch %value, ^default, 0:^case_0, 1:^case_0, 2:^case_0, 3:^case_0, 4:^case_1, 5:^case_1, 6:^case_1, 7:^case_1, 8:^case_2, 9:^case_2, 10:^case_2, 11:^case_2, 12:^case_2, 13:^case_2, 14:^case_2, 15:^case_2

  block ^case_0:
    return i64 11

  block ^case_1:
    return i64 22

  block ^case_2:
    return i64 33

  block ^default:
    return i64 99
}

function @gapped(%value : i32) -> i64 [binding=strong] {
  block ^entry:
    switch %value, ^default, 0:^case_0, 1:^case_0, 2:^case_0, 3:^case_0, 8:^case_1, 9:^case_1, 10:^case_1, 11:^case_1

  block ^case_0:
    return i64 11

  block ^case_1:
    return i64 22

  block ^default:
    return i64 99
}

function @signed_cases(%value : i32) -> i64 [binding=strong] {
  block ^entry:
    switch %value, ^default, -4:^case_0, -3:^case_0, -2:^case_0, -1:^case_0, 0:^case_1, 1:^case_1, 2:^case_1, 3:^case_1

  block ^case_0:
    return i64 11

  block ^case_1:
    return i64 22

  block ^default:
    return i64 99
}

function @unsigned_prefix(%value : u32) -> i64 [binding=strong] {
  block ^entry:
    switch %value, ^default, 0:^case_0, 1:^case_0, 2:^case_0, 3:^case_0, 4:^case_1, 5:^case_1, 6:^case_1, 7:^case_1

  block ^case_0:
    return i64 11

  block ^case_1:
    return i64 22

  block ^default:
    return i64 99
}

function @wide_prefix(%value : i64) -> i64 [binding=strong] {
  block ^entry:
    switch %value, ^default, 0:^case_0, 1:^case_0, 2:^case_0, 3:^case_0, 4:^case_1, 5:^case_1, 6:^case_1, 7:^case_1

  block ^case_0:
    return i64 11

  block ^case_1:
    return i64 22

  block ^default:
    return i64 99
}

function @byte_prefix(%value : u8) -> i64 [binding=strong] {
  block ^entry:
    switch %value, ^default, 0:^case_0, 1:^case_0, 2:^case_0, 3:^case_0, 4:^case_1, 5:^case_1, 6:^case_1, 7:^case_1

  block ^case_0:
    return i64 11

  block ^case_1:
    return i64 22

  block ^default:
    return i64 99
}

function @main() -> i32 [role=entry, binding=strong] {
  block ^entry:
    %v0 = call i64 @signed_prefix(-2147483648)
    %bad0 = cmp ne i64 %v0, 99
    %all0 = copy i64 %bad0
    %v1 = call i64 @signed_prefix(-1)
    %bad1 = cmp ne i64 %v1, 99
    %all1 = binary or i64 %all0, %bad1
    %v2 = call i64 @signed_prefix(0)
    %bad2 = cmp ne i64 %v2, 11
    %all2 = binary or i64 %all1, %bad2
    %v3 = call i64 @signed_prefix(3)
    %bad3 = cmp ne i64 %v3, 11
    %all3 = binary or i64 %all2, %bad3
    %v4 = call i64 @signed_prefix(4)
    %bad4 = cmp ne i64 %v4, 22
    %all4 = binary or i64 %all3, %bad4
    %v5 = call i64 @signed_prefix(7)
    %bad5 = cmp ne i64 %v5, 22
    %all5 = binary or i64 %all4, %bad5
    %v6 = call i64 @signed_prefix(8)
    %bad6 = cmp ne i64 %v6, 33
    %all6 = binary or i64 %all5, %bad6
    %v7 = call i64 @signed_prefix(15)
    %bad7 = cmp ne i64 %v7, 33
    %all7 = binary or i64 %all6, %bad7
    %v8 = call i64 @signed_prefix(16)
    %bad8 = cmp ne i64 %v8, 99
    %all8 = binary or i64 %all7, %bad8
    %v9 = call i64 @signed_prefix(2147483647)
    %bad9 = cmp ne i64 %v9, 99
    %all9 = binary or i64 %all8, %bad9
    %v10 = call i64 @gapped(-1)
    %bad10 = cmp ne i64 %v10, 99
    %all10 = binary or i64 %all9, %bad10
    %v11 = call i64 @gapped(0)
    %bad11 = cmp ne i64 %v11, 11
    %all11 = binary or i64 %all10, %bad11
    %v12 = call i64 @gapped(3)
    %bad12 = cmp ne i64 %v12, 11
    %all12 = binary or i64 %all11, %bad12
    %v13 = call i64 @gapped(4)
    %bad13 = cmp ne i64 %v13, 99
    %all13 = binary or i64 %all12, %bad13
    %v14 = call i64 @gapped(7)
    %bad14 = cmp ne i64 %v14, 99
    %all14 = binary or i64 %all13, %bad14
    %v15 = call i64 @gapped(8)
    %bad15 = cmp ne i64 %v15, 22
    %all15 = binary or i64 %all14, %bad15
    %v16 = call i64 @gapped(11)
    %bad16 = cmp ne i64 %v16, 22
    %all16 = binary or i64 %all15, %bad16
    %v17 = call i64 @gapped(12)
    %bad17 = cmp ne i64 %v17, 99
    %all17 = binary or i64 %all16, %bad17
    %v18 = call i64 @signed_cases(-2147483648)
    %bad18 = cmp ne i64 %v18, 99
    %all18 = binary or i64 %all17, %bad18
    %v19 = call i64 @signed_cases(-5)
    %bad19 = cmp ne i64 %v19, 99
    %all19 = binary or i64 %all18, %bad19
    %v20 = call i64 @signed_cases(-4)
    %bad20 = cmp ne i64 %v20, 11
    %all20 = binary or i64 %all19, %bad20
    %v21 = call i64 @signed_cases(-1)
    %bad21 = cmp ne i64 %v21, 11
    %all21 = binary or i64 %all20, %bad21
    %v22 = call i64 @signed_cases(0)
    %bad22 = cmp ne i64 %v22, 22
    %all22 = binary or i64 %all21, %bad22
    %v23 = call i64 @signed_cases(3)
    %bad23 = cmp ne i64 %v23, 22
    %all23 = binary or i64 %all22, %bad23
    %v24 = call i64 @signed_cases(4)
    %bad24 = cmp ne i64 %v24, 99
    %all24 = binary or i64 %all23, %bad24
    %v25 = call i64 @unsigned_prefix(0)
    %bad25 = cmp ne i64 %v25, 11
    %all25 = binary or i64 %all24, %bad25
    %v26 = call i64 @unsigned_prefix(3)
    %bad26 = cmp ne i64 %v26, 11
    %all26 = binary or i64 %all25, %bad26
    %v27 = call i64 @unsigned_prefix(4)
    %bad27 = cmp ne i64 %v27, 22
    %all27 = binary or i64 %all26, %bad27
    %v28 = call i64 @unsigned_prefix(7)
    %bad28 = cmp ne i64 %v28, 22
    %all28 = binary or i64 %all27, %bad28
    %v29 = call i64 @unsigned_prefix(8)
    %bad29 = cmp ne i64 %v29, 99
    %all29 = binary or i64 %all28, %bad29
    %v30 = call i64 @unsigned_prefix(2147483648)
    %bad30 = cmp ne i64 %v30, 99
    %all30 = binary or i64 %all29, %bad30
    %v31 = call i64 @unsigned_prefix(4294967295)
    %bad31 = cmp ne i64 %v31, 99
    %all31 = binary or i64 %all30, %bad31
    %v32 = call i64 @wide_prefix(-1)
    %bad32 = cmp ne i64 %v32, 99
    %all32 = binary or i64 %all31, %bad32
    %v33 = call i64 @wide_prefix(0)
    %bad33 = cmp ne i64 %v33, 11
    %all33 = binary or i64 %all32, %bad33
    %v34 = call i64 @wide_prefix(7)
    %bad34 = cmp ne i64 %v34, 22
    %all34 = binary or i64 %all33, %bad34
    %v35 = call i64 @wide_prefix(8)
    %bad35 = cmp ne i64 %v35, 99
    %all35 = binary or i64 %all34, %bad35
    %v36 = call i64 @wide_prefix(4294967296)
    %bad36 = cmp ne i64 %v36, 99
    %all36 = binary or i64 %all35, %bad36
    %v37 = call i64 @wide_prefix(9223372036854775807)
    %bad37 = cmp ne i64 %v37, 99
    %all37 = binary or i64 %all36, %bad37
    %v38 = call i64 @byte_prefix(0)
    %bad38 = cmp ne i64 %v38, 11
    %all38 = binary or i64 %all37, %bad38
    %v39 = call i64 @byte_prefix(3)
    %bad39 = cmp ne i64 %v39, 11
    %all39 = binary or i64 %all38, %bad39
    %v40 = call i64 @byte_prefix(4)
    %bad40 = cmp ne i64 %v40, 22
    %all40 = binary or i64 %all39, %bad40
    %v41 = call i64 @byte_prefix(7)
    %bad41 = cmp ne i64 %v41, 22
    %all41 = binary or i64 %all40, %bad41
    %v42 = call i64 @byte_prefix(8)
    %bad42 = cmp ne i64 %v42, 99
    %all42 = binary or i64 %all41, %bad42
    %v43 = call i64 @byte_prefix(255)
    %bad43 = cmp ne i64 %v43, 99
    %all43 = binary or i64 %all42, %bad43
    %result = convert trunc i32 i64 %all43
    return i32 %result
}
