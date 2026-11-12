# An unswitched loop whose test follows a block of the loop: the copy keeps
# each definition before its uses (the latch reads %scaled of ^mid).
global @values : obj<32x8> = { i64 1 i64 2 i64 3 i64 4 }
global @odd : obj<8x8> = { i64 0 }
global @even : obj<8x8> = { i64 0 }

function @shape(%flag : i64, %p : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^header

  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %sum = phi i64 [^entry: 0, ^latch: %total]
    %at = index i64 %p, %i
    %v = load i64 %at
    %low = binary and i64 %v, 1
    branch %low, ^left, ^right

  block ^left:
    store i64 %v, @odd
    jump ^mid

  block ^right:
    store i64 %i, @even
    jump ^mid

  block ^mid:
    %scaled = binary mul i64 %v, 3
    branch %flag, ^a, ^b

  block ^a:
    store i64 %scaled, %at
    jump ^latch

  block ^b:
    store i64 %i, @even
    jump ^latch

  block ^latch:
    %total = binary add i64 %sum, %scaled
    %next = binary add i64 %i, 1
    %more = cmp lt i64 %next, %n
    branch %more, ^header, ^exit

  block ^exit:
    return i64 %total
}

function @main() -> i32 {
  block ^entry:
    %kept = call i64 @shape(0, @values, 4)
    %stored = call i64 @shape(1, @values, 4)
    %again = call i64 @shape(0, @values, 4)
    %kept_ok = cmp eq i64 %kept, 30
    %stored_ok = cmp eq i64 %stored, 30
    %again_ok = cmp eq i64 %again, 90
    %kept_bit = select i32 %kept_ok, 0, 1
    %stored_bit = select i32 %stored_ok, 0, 2
    %again_bit = select i32 %again_ok, 0, 4
    %some = binary or i32 %kept_bit, %stored_bit
    %status = binary or i32 %some, %again_bit
    return i32 %status
}
