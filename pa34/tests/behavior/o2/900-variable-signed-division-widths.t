# Signed 64-bit quotients and remainders, with operands of either sign on both
# sides of the 32-bit limits.
function @quotient_and_remainder(%a : i64, %b : i64) -> i64 {
  block ^entry:
    %q = binary div i64 %a, %b
    %r = binary mod i64 %a, %b
    %answer = binary xor i64 %q, %r
    return i64 %answer
}
function @main() -> i32 [role=entry] {
  block ^entry:
    %v0 = call i64 @quotient_and_remainder(0, 1)
    %bad0 = cmp ne i64 %v0, 0
    %v1 = call i64 @quotient_and_remainder(1, 1)
    %bad1 = cmp ne i64 %v1, 1
    %all1 = binary or i1 %bad0, %bad1
    %v2 = call i64 @quotient_and_remainder(-1, 1)
    %bad2 = cmp ne i64 %v2, -1
    %all2 = binary or i1 %all1, %bad2
    %v3 = call i64 @quotient_and_remainder(7, -2)
    %bad3 = cmp ne i64 %v3, -4
    %all3 = binary or i1 %all2, %bad3
    %v4 = call i64 @quotient_and_remainder(-7, 2)
    %bad4 = cmp ne i64 %v4, 2
    %all4 = binary or i1 %all3, %bad4
    %v5 = call i64 @quotient_and_remainder(2147483647, 2)
    %bad5 = cmp ne i64 %v5, 1073741822
    %all5 = binary or i1 %all4, %bad5
    %v6 = call i64 @quotient_and_remainder(2147483648, 3)
    %bad6 = cmp ne i64 %v6, 715827880
    %all6 = binary or i1 %all5, %bad6
    %v7 = call i64 @quotient_and_remainder(4294967295, 2147483648)
    %bad7 = cmp ne i64 %v7, 2147483646
    %all7 = binary or i1 %all6, %bad7
    %v8 = call i64 @quotient_and_remainder(4294967295, 4294967295)
    %bad8 = cmp ne i64 %v8, 1
    %all8 = binary or i1 %all7, %bad8
    %v9 = call i64 @quotient_and_remainder(3000000000, 7)
    %bad9 = cmp ne i64 %v9, 428571424
    %all9 = binary or i1 %all8, %bad9
    %v10 = call i64 @quotient_and_remainder(-3000000000, 7)
    %bad10 = cmp ne i64 %v10, 428571424
    %all10 = binary or i1 %all9, %bad10
    %v11 = call i64 @quotient_and_remainder(3000000000, -7)
    %bad11 = cmp ne i64 %v11, -428571432
    %all11 = binary or i1 %all10, %bad11
    %v12 = call i64 @quotient_and_remainder(4294967296, 3)
    %bad12 = cmp ne i64 %v12, 1431655764
    %all12 = binary or i1 %all11, %bad12
    %v13 = call i64 @quotient_and_remainder(17, 4294967296)
    %bad13 = cmp ne i64 %v13, 17
    %all13 = binary or i1 %all12, %bad13
    %v14 = call i64 @quotient_and_remainder(0, -5)
    %bad14 = cmp ne i64 %v14, 0
    %all14 = binary or i1 %all13, %bad14
    %v15 = call i64 @quotient_and_remainder(-1, -1)
    %bad15 = cmp ne i64 %v15, 1
    %all15 = binary or i1 %all14, %bad15
    %v16 = call i64 @quotient_and_remainder(4294967295, -1)
    %bad16 = cmp ne i64 %v16, -4294967295
    %all16 = binary or i1 %all15, %bad16
    %v17 = call i64 @quotient_and_remainder(-4611686018427387904, 4294967296)
    %bad17 = cmp ne i64 %v17, -1073741824
    %all17 = binary or i1 %all16, %bad17
    %v18 = call i64 @quotient_and_remainder(-9223372036854775808, 4294967296)
    %bad18 = cmp ne i64 %v18, -2147483648
    %all18 = binary or i1 %all17, %bad18
    %v19 = call i64 @quotient_and_remainder(9223372036854775807, 4294967295)
    %bad19 = cmp ne i64 %v19, 4294967295
    %all19 = binary or i1 %all18, %bad19
    %v20 = call i64 @quotient_and_remainder(-9223372036854775807, 2)
    %bad20 = cmp ne i64 %v20, 4611686018427387902
    %all20 = binary or i1 %all19, %bad20
    %out = convert zext i32 i1 %all20
    return i32 %out
}
