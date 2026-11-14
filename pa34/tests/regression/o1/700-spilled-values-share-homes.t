# Values spilled across the calls of one block and dead before the next
# block give their frame homes to the values spilled there: the frame of
# @twice holds the homes one block needs, not those of both. Ten values are
# live across the first call of @note in each block, more than the registers
# a call preserves.
global @table : obj<160x8> = { i64 1 i64 2 i64 3 i64 4 i64 5 i64 6 i64 7 i64 8 i64 9 i64 10 i64 11 i64 12 i64 13 i64 14 i64 15 i64 16 i64 17 i64 18 i64 19 i64 20 }
global @seen : i64 = 0

function @note(%v : i64) -> void [unwind=no, no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %new = binary add i64 %old, %v
    store i64 %new, @seen
    return void
}

function @twice(%p : ptr) -> void [unwind=no, no_inline=yes] {
  block ^entry:
    %at_a0 = index i64 [projection=array_element] %p, 0
    %a0 = load i64 %at_a0
    %at_a1 = index i64 [projection=array_element] %p, 1
    %a1 = load i64 %at_a1
    %at_a2 = index i64 [projection=array_element] %p, 2
    %a2 = load i64 %at_a2
    %at_a3 = index i64 [projection=array_element] %p, 3
    %a3 = load i64 %at_a3
    %at_a4 = index i64 [projection=array_element] %p, 4
    %a4 = load i64 %at_a4
    %at_a5 = index i64 [projection=array_element] %p, 5
    %a5 = load i64 %at_a5
    %at_a6 = index i64 [projection=array_element] %p, 6
    %a6 = load i64 %at_a6
    %at_a7 = index i64 [projection=array_element] %p, 7
    %a7 = load i64 %at_a7
    %at_a8 = index i64 [projection=array_element] %p, 8
    %a8 = load i64 %at_a8
    %at_a9 = index i64 [projection=array_element] %p, 9
    %a9 = load i64 %at_a9
    call void @note(%a0)
    call void @note(%a1)
    call void @note(%a2)
    call void @note(%a3)
    call void @note(%a4)
    call void @note(%a5)
    call void @note(%a6)
    call void @note(%a7)
    call void @note(%a8)
    call void @note(%a9)
    jump ^later

  block ^later:
    %at_b0 = index i64 [projection=array_element] %p, 10
    %b0 = load i64 %at_b0
    %at_b1 = index i64 [projection=array_element] %p, 11
    %b1 = load i64 %at_b1
    %at_b2 = index i64 [projection=array_element] %p, 12
    %b2 = load i64 %at_b2
    %at_b3 = index i64 [projection=array_element] %p, 13
    %b3 = load i64 %at_b3
    %at_b4 = index i64 [projection=array_element] %p, 14
    %b4 = load i64 %at_b4
    %at_b5 = index i64 [projection=array_element] %p, 15
    %b5 = load i64 %at_b5
    %at_b6 = index i64 [projection=array_element] %p, 16
    %b6 = load i64 %at_b6
    %at_b7 = index i64 [projection=array_element] %p, 17
    %b7 = load i64 %at_b7
    %at_b8 = index i64 [projection=array_element] %p, 18
    %b8 = load i64 %at_b8
    %at_b9 = index i64 [projection=array_element] %p, 19
    %b9 = load i64 %at_b9
    call void @note(%b0)
    call void @note(%b1)
    call void @note(%b2)
    call void @note(%b3)
    call void @note(%b4)
    call void @note(%b5)
    call void @note(%b6)
    call void @note(%b7)
    call void @note(%b8)
    call void @note(%b9)
    return void
}

# 1 + ... + 20 is 210.
function @main() -> i64 [role=entry] {
  block ^entry:
    call void @twice(@table)
    %seen = load i64 @seen
    %status = binary sub i64 %seen, 210
    return i64 %status
}
