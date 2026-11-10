declare function @query(%leaf : i32, %subleaf : i32, %output : ptr) -> void [role=cpu_query, unwind=no]
function @main() -> i32 [role=entry] {
  slot $first : obj<32x8>
  slot $second : obj<32x8>
  block ^entry:
    %first = addr $first
    %second = addr $second
    %a = index i8 %first, 8
    %a1 = index i8 %first, 16
    %aend = index i8 %first, 24
    %b = index i8 %second, 8
    %b1 = index i8 %second, 16
    %bend = index i8 %second, 24
    store i64 31, %first
    store i64 37, %aend
    store i64 41, %second
    store i64 43, %bend
    store i64 0, %a
    store i64 0, %a1
    store i64 1, %b
    store i64 1, %b1
    call void @query(0, 0, %a)
    call void @query(0, 0, %b)
    %av = load i64 %a
    %av1 = load i64 %a1
    %bv = load i64 %b
    %bv1 = load i64 %b1
    %ag = load i64 %first
    %ag1 = load i64 %aend
    %bg = load i64 %second
    %bg1 = load i64 %bend
    %bad0 = cmp ne i64 %av, %bv
    %bad1 = cmp ne i64 %av1, %bv1
    %bad2 = cmp ne i64 %ag, 31
    %bad3 = cmp ne i64 %ag1, 37
    %bad4 = cmp ne i64 %bg, 41
    %bad5 = cmp ne i64 %bg1, 43
    %sum0 = binary or i1 %bad0, %bad1
    %sum1 = binary or i1 %sum0, %bad2
    %sum2 = binary or i1 %sum1, %bad3
    %sum3 = binary or i1 %sum2, %bad4
    %sum4 = binary or i1 %sum3, %bad5
    %exit = convert zext i32 i1 %sum4
    return i32 %exit
}
