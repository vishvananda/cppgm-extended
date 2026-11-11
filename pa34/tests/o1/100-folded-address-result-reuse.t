global @cells = {
  ptr 11
  ptr 37
  ptr 73
}

function @masked_index(%base : ptr, %tag : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %shift = binary ushr i64 %tag, 1
    %mask = binary and i64 %shift, 8
    %offset = binary add i64 %mask, 8
    %at = index i8 %base, %offset
    %next = load ptr %at
    jump ^finish
  block ^finish:
    %base_bits = copy i64 %base
    %next_bits = copy i64 %next
    %mixed = binary xor i64 %base_bits, %tag
    %result = binary xor i64 %mixed, %next_bits
    return i64 %result
}

function @live_index(%base : ptr, %tag : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %shift = binary ushr i64 %tag, 1
    %mask = binary and i64 %shift, 8
    %offset = binary add i64 %mask, 8
    %at = index i8 %base, %offset
    %next = load ptr %at
    jump ^finish
  block ^finish:
    %next_bits = copy i64 %next
    %result = binary xor i64 %mask, %next_bits
    return i64 %result
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %base = addr @cells
    %bits = copy i64 %base
    %a = call i64 @masked_index(%base, 4)
    %a0 = binary xor i64 %bits, 4
    %want_a = binary xor i64 %a0, 37
    %bad_a = cmp ne i64 %a, %want_a
    %b = call i64 @masked_index(%base, 16)
    %b0 = binary xor i64 %bits, 16
    %want_b = binary xor i64 %b0, 73
    %bad_b = cmp ne i64 %b, %want_b
    %c = call i64 @live_index(%base, 4)
    %bad_c = cmp ne i64 %c, 37
    %d = call i64 @live_index(%base, 16)
    %bad_d = cmp ne i64 %d, 65
    %ab = binary or i64 %bad_a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %bad = binary or i64 %ab, %cd
    %status = convert trunc i32 i64 %bad
    return i32 %status
}
