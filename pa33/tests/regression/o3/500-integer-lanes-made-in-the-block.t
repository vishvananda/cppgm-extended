# Four adjacent u32 stores of values computed in their block, each a
# trailing-zero count of a mask with its lowest bit cleared step by step.
# Every lane would need a move into a vector register before its insert.
function @write_index(%tail : ptr, %idx : u32, %bits : i64) -> void [no_inline=yes] {
  block ^entry:
    %z0 = unary ctz i64 %bits
    %n0 = convert trunc u32 i64 %z0
    %v0 = binary add u32 %idx, %n0
    store typed u32 %v0, %tail
    %m0 = binary sub i64 %bits, 1
    %b1 = binary and i64 %bits, %m0
    %z1 = unary ctz i64 %b1
    %n1 = convert trunc u32 i64 %z1
    %v1 = binary add u32 %idx, %n1
    %p1 = index u32 %tail, 1
    store typed u32 %v1, %p1
    %m1 = binary sub i64 %b1, 1
    %b2 = binary and i64 %b1, %m1
    %z2 = unary ctz i64 %b2
    %n2 = convert trunc u32 i64 %z2
    %v2 = binary add u32 %idx, %n2
    %p2 = index u32 %tail, 2
    store typed u32 %v2, %p2
    %m2 = binary sub i64 %b2, 1
    %b3 = binary and i64 %b2, %m2
    %z3 = unary ctz i64 %b3
    %n3 = convert trunc u32 i64 %z3
    %v3 = binary add u32 %idx, %n3
    %p3 = index u32 %tail, 3
    store typed u32 %v3, %p3
    return void
}

function @main() -> i32 [role=entry] {
  slot $tail : obj<16x4>
  block ^entry:
    %t = addr $tail
    call void @write_index(%t, 100, 4398048616704)
    %a = load typed u32 %t
    %p1 = index u32 %t, 1
    %b = load typed u32 %p1
    %p2 = index u32 %t, 2
    %c = load typed u32 %p2
    %p3 = index u32 %t, 3
    %d = load typed u32 %p3
    %ab = binary mul u32 %a, 1000
    %abc = binary add u32 %ab, %b
    %x = binary xor u32 %abc, %c
    %y = binary xor u32 %x, %d
    %e = cmp eq u32 %y, 108198
    %r = select i32 %e, 0, 1
    return i32 %r
}
