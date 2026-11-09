function @swap(%a : ptr [alias=noalias], %b : ptr [alias=noalias]) -> void [no_inline=yes] {
  block ^entry:
    %a1 = index i8 %a, 4
    %b1 = index i8 %b, 4
    %v0 = load i32 %b
    %v1 = load i32 %b1
    %v2 = load i32 %a
    %v3 = load i32 %a1
    store i32 %v0, %a
    store i32 %v1, %a1
    store i32 %v2, %b
    store i32 %v3, %b1
    return void
}
function @main() -> i32 [role=entry] {
  slot $a : obj<8x4>
  slot $b : obj<8x4>
  block ^entry:
    %a = addr $a
    %a1 = index i8 %a, 4
    %b = addr $b
    %b1 = index i8 %b, 4
    store i32 3, %a
    store i32 5, %a1
    store i32 7, %b
    store i32 9, %b1
    call void @swap(%a, %b)
    %av0 = load i32 %a
    %av1 = load i32 %a1
    %bv0 = load i32 %b
    %bv1 = load i32 %b1
    %bad0 = cmp ne i32 %av0, 7
    %bad1 = cmp ne i32 %av1, 9
    %bad2 = cmp ne i32 %bv0, 3
    %bad3 = cmp ne i32 %bv1, 5
    %bad01 = binary or u8 %bad0, %bad1
    %bad23 = binary or u8 %bad2, %bad3
    %bad = binary or u8 %bad01, %bad23
    %code = convert zext i32 u8 %bad
    return i32 %code
}
