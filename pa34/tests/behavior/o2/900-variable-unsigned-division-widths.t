function @quotient_and_remainder(%a : i64, %b : i64) -> i64 {
  block ^entry:
    %q = binary udiv i64 %a, %b
    %r = binary umod i64 %a, %b
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
    %v2 = call i64 @quotient_and_remainder(4294967295, 1)
    %bad2 = cmp ne i64 %v2, 4294967295
    %all2 = binary or i1 %all1, %bad2
    %v3 = call i64 @quotient_and_remainder(4294967295, 2147483648)
    %bad3 = cmp ne i64 %v3, 2147483646
    %all3 = binary or i1 %all2, %bad3
    %v4 = call i64 @quotient_and_remainder(4294967296, 3)
    %bad4 = cmp ne i64 %v4, 1431655764
    %all4 = binary or i1 %all3, %bad4
    %v5 = call i64 @quotient_and_remainder(9223372036854775808, 17)
    %bad5 = cmp ne i64 %v5, 542551296285575054
    %all5 = binary or i1 %all4, %bad5
    %v6 = call i64 @quotient_and_remainder(18446744073709551615, 18446744073709551615)
    %bad6 = cmp ne i64 %v6, 1
    %all6 = binary or i1 %all5, %bad6
    %v7 = call i64 @quotient_and_remainder(18446744073709551615, 4294967297)
    %bad7 = cmp ne i64 %v7, 4294967295
    %all7 = binary or i1 %all6, %bad7
    %v8 = call i64 @quotient_and_remainder(18446744073709551615, 3)
    %bad8 = cmp ne i64 %v8, 6148914691236517205
    %all8 = binary or i1 %all7, %bad8
    %v9 = call i64 @quotient_and_remainder(17, 4294967296)
    %bad9 = cmp ne i64 %v9, 17
    %all9 = binary or i1 %all8, %bad9
    %v10 = call i64 @quotient_and_remainder(4294967296, 4294967296)
    %bad10 = cmp ne i64 %v10, 1
    %all10 = binary or i1 %all9, %bad10
    %out = convert zext i32 i1 %all10
    return i32 %out
}
