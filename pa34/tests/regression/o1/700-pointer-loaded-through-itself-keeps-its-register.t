# A pointer loaded through itself and read in a later block keeps its
# register: the load reads its address before it writes its result, so the
# next link takes the register of the link it was read through, and no edge
# of the loop moves one into the other.
global @nodes : obj<72x8> = { zero 72 }

function @marked_sum(%p : ptr, %n : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %cur = phi ptr [^entry: %p, ^latch: %next]
    %i = phi i64 [^entry: 0, ^latch: %i1]
    %sum = phi i64 [^entry: 0, ^latch: %sum2]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^done

  block ^body:
    %at = index i8 %cur, 8
    %next = load ptr %at
    %flag_at = index i8 %next, 16
    %flag = load u8 %flag_at
    branch %flag, ^marked, ^latch

  block ^marked:
    %v = load i64 %next
    %sum1 = binary add i64 %sum, %v
    jump ^latch

  block ^latch:
    %sum2 = phi i64 [^body: %sum, ^marked: %sum1]
    %i1 = binary add i64 %i, 1
    jump ^head

  block ^done:
    return i64 %sum
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %n0 = addr @nodes
    %n1 = index i8 %n0, 24
    %n2 = index i8 %n0, 48
    store i64 5, %n0
    store i64 7, %n1
    store i64 11, %n2
    %next0 = index i8 %n0, 8
    store ptr %n1, %next0
    %next1 = index i8 %n1, 8
    store ptr %n2, %next1
    %next2 = index i8 %n2, 8
    store ptr %n0, %next2
    %flag1 = index i8 %n1, 16
    store u8 1, %flag1
    %flag2 = index i8 %n2, 16
    store u8 1, %flag2
    %sum = call i64 @marked_sum(%n0, 4)
    %bad = cmp ne i64 %sum, 25
    %status = convert zext i64 i1 %bad
    return i64 %status
}
