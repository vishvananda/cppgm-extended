# A loop whose first test goes on to the step either way (a `continue`). The
# test's copy at the step closes a cycle that the first test enters in two
# places, and the copy's branch back to the step stays in the cycle, as its
# other side does. Such a branch is weighed by what it compares (an equality
# with a constant is the rarer outcome), not as a loop's back edge: the body
# follows the copy, and a pass through the body takes one branch.
global @rows : obj<32x4> [binding=internal] = {
  u32 5
  u32 1
  u32 6
  u32 4294967295
  u32 7
  u32 0
  u32 8
  u32 1
}
global @out : obj<8x4> [binding=internal] = {
  zero 8
}
global @count : obj<8x4> [binding=internal] = {
  zero 8
}

function @map(%rows : ptr, %n : i64, %out : ptr, %count : ptr) -> void [binding=internal, no_inline=yes] {
  block ^entry:
    branch %n, ^body, ^done

  block ^body:
    %i = phi i64 [^entry: 0, ^step: %next]
    %scaled = binary mul i64 %i, 8
    %row = index i8 %rows, %scaled
    %field = index i8 %row, 4
    %block = load typed u32 %field
    %skip = cmp eq u32 %block, 4294967295
    branch %skip, ^step, ^record

  block ^record:
    %narrow = convert trunc u32 i64 %i
    %place = index u32 %out, %block
    store typed u32 %narrow, %place
    %again = load typed u32 %field
    %counter = index u32 %count, %again
    %old = load typed u32 %counter
    %new = binary add u32 %old, 1
    store typed u32 %new, %counter
    jump ^step

  block ^step:
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^body, ^done

  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    call void @map(@rows, 4, @out, @count)
    %second = index i8 @out, 4
    %last = load typed u32 %second
    %counted_at = index i8 @count, 4
    %counted = load typed u32 %counted_at
    %scaled = binary mul u32 %last, 10
    %sum = binary add u32 %scaled, %counted
    %result = copy i32 %sum
    return i32 %result
}
