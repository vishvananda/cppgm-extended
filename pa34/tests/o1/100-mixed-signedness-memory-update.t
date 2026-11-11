# Integer views of the same width store the same updated low bits. Boolean
# reads, volatile accesses, snapshots and separately used old values differ.
function @byte_update(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    %old = load u8 %p
    %next = binary add u8 %old, 1
    store i8 %next, %p
    return void
}

function @word_update(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    %old = load i16 %p
    %next = binary xor i16 %old, 32768
    store u16 %next, %p
    return void
}

function @dword_update(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    %old = load u32 %p
    %next = binary sub u32 %old, 1
    store i32 %next, %p
    return void
}

function @wide_update(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    %old = load u16 %p
    %wide = convert zext i64 u16 %old
    %next = binary add i64 %wide, 65537
    %narrow = convert trunc u16 i64 %next
    store i16 %narrow, %p
    return void
}

function @truth_update(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    %old = load u8 %p
    %bit = convert trunc i1 u8 %old
    %byte = convert zext u8 i1 %bit
    %next = binary add u8 %byte, 1
    store i8 %next, %p
    return void
}

function @volatile_update(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    %old = load volatile u8 %p
    %next = binary add u8 %old, 1
    store volatile i8 %next, %p
    return void
}

function @snapshot_update(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    %old = load u8 %p
    store i8 123, %p
    %next = binary add u8 %old, 1
    store i8 %next, %p
    return void
}

function @live_old_value(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %old = load u8 %p
    %next = binary add u8 %old, 1
    store i8 %next, %p
    %result = convert zext i64 u8 %old
    return i64 %result
}

function @intervening_observation(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    %old = load u8 %p
    %observed = load volatile u8 %p
    %next = binary add u8 %old, 1
    store i8 %next, %p
    return void
}

function @main() -> i32 [role=entry] {
  slot $cell : i64
  block ^entry:
    %p = addr $cell
    store i64 511, %p
    call void @byte_update(%p)
    %byte = load i64 %p
    %bad_byte = cmp ne i64 %byte, 256
    store i64 65535, %p
    call void @word_update(%p)
    %word = load i64 %p
    %bad_word = cmp ne i64 %word, 32767
    store i64 4294967296, %p
    call void @dword_update(%p)
    %dword = load i64 %p
    %bad_dword = cmp ne i64 %dword, 8589934591
    store i64 131071, %p
    call void @wide_update(%p)
    %wide = load i64 %p
    %bad_wide = cmp ne i64 %wide, 65536
    store i64 254, %p
    call void @truth_update(%p)
    %truth = load i64 %p
    %bad_truth = cmp ne i64 %truth, 1
    store i64 255, %p
    call void @volatile_update(%p)
    %vol = load i64 %p
    %bad_vol = cmp ne i64 %vol, 0
    store i64 254, %p
    call void @snapshot_update(%p)
    %snapshot = load i64 %p
    %bad_snapshot = cmp ne i64 %snapshot, 255
    store i64 255, %p
    %old = call i64 @live_old_value(%p)
    %new = load i64 %p
    %bad_old = cmp ne i64 %old, 255
    %bad_new = cmp ne i64 %new, 0
    store i64 255, %p
    call void @intervening_observation(%p)
    %observed = load i64 %p
    %bad_observed = cmp ne i64 %observed, 0
    %a = binary or i64 %bad_byte, %bad_word
    %b = binary or i64 %bad_dword, %bad_wide
    %c = binary or i64 %bad_truth, %bad_vol
    %d = binary or i64 %bad_snapshot, %bad_old
    %ab = binary or i64 %a, %b
    %cd = binary or i64 %c, %d
    %abcd = binary or i64 %ab, %cd
    %bad = binary or i64 %abcd, %bad_new
    %with_observation = binary or i64 %bad, %bad_observed
    %status = convert trunc i32 i64 %with_observation
    return i32 %status
}
