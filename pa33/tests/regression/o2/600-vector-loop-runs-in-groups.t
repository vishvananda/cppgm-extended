# A short loop of vector operations runs two passes for each test, as a loop
# of scalar operations does. It stays while its counter is at or before a
# literal: from a literal first value the distance to the next value past the
# bound is whole steps, so no pass is left for a loop after the groups. The
# second pass reads at a displacement from the first pass's address.
global @words : obj<64x4> = {
  zero 64
}

function @masked_sum(%mask : v128) -> v128 [binding=strong, no_inline=yes] {
  block ^entry:
    %base = addr @words
    %zero = vector pzero
    jump ^loop
  block ^loop:
    %sum = phi v128 [^entry: %zero, ^loop: %next_sum]
    %i = phi i64 [^entry: 0, ^loop: %next]
    %at = index inbounds u32 [projection=array_element] %base, %i
    %four = load v128 %at
    %kept = vector pand %four, %mask
    %next_sum = vector paddd %sum, %kept
    %next = binary add nowrap i64 %i, 4
    %narrow = convert trunc exact u32 i64 %next
    %more = cmp ule u32 %narrow, 60
    branch %more, ^loop, ^done
  block ^done:
    return v128 %next_sum
}

# The counter is narrower than the address: its extension in the second pass
# is the first pass's plus the step, because the passes of a group stay
# before the bound.
function @narrow_counter(%mask : v128, %count : u32) -> v128 [binding=strong, no_inline=yes] {
  block ^entry:
    %base = addr @words
    %zero = vector pzero
    jump ^loop
  block ^loop:
    %sum = phi v128 [^entry: %zero, ^loop: %next_sum]
    %i = phi u32 [^entry: 0, ^loop: %next]
    %wide = convert zext i64 u32 %i
    %at = index inbounds i32 %base, %wide
    %four = load v128 %at
    %kept = vector pxor %four, %mask
    %next_sum = vector paddd %sum, %kept
    %next = binary add nowrap u32 %i, 4
    %more = cmp ult u32 %next, %count
    branch %more, ^loop, ^done
  block ^done:
    return v128 %next_sum
}
