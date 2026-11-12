# A loop's test of two values it does not compute stays beside the calls in
# the loop; unswitching makes that compare in the preheader, whose result is
# i64.
global @values : obj<32x8> = { i64 5 i64 6 i64 7 i64 8 }
global @seen : obj<8x8> = { i64 0 }

function @note(%v : i64) -> void [no_inline=yes, unwind=no] {
  block ^entry:
    %old = load i64 @seen
    %sum = binary add i64 %old, %v
    store i64 %sum, @seen
    return void
}

function @narrow_test(%a : i32, %b : i32, %p : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^header

  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %at = index i64 %p, %i
    %v = load i64 %at
    %less = cmp lt i32 %a, %b
    branch %less, ^value, ^place

  block ^value:
    call void @note(%v)
    jump ^latch

  block ^place:
    %scaled = binary mul i64 %i, 100
    call void @note(%scaled)
    jump ^latch

  block ^latch:
    %next = binary add i64 %i, 1
    %more = cmp lt i64 %next, %n
    branch %more, ^header, ^exit

  block ^exit:
    return void
}

function @main() -> i32 {
  block ^entry:
    call void @narrow_test(1, 2, @values, 4)
    %values = load i64 @seen
    call void @narrow_test(2, 1, @values, 4)
    %both = load i64 @seen
    %values_ok = cmp eq i64 %values, 26
    %both_ok = cmp eq i64 %both, 626
    %values_bit = select i32 %values_ok, 0, 1
    %both_bit = select i32 %both_ok, 0, 2
    %status = binary or i32 %values_bit, %both_bit
    return i32 %status
}
