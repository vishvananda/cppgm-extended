global @source : obj<8x4> = zero

# An object load captures its value before a later store changes the source.
function @snapshot() -> obj<8x4> [no_inline=yes] {
  block ^entry:
    %p = addr @source
    store i32 7, %p
    %saved = load obj<8x4> @source
    store i32 9, %p
    return obj<8x4> %saved
}

# Packing a word does not extend or truncate a same-width signedness change.
function @word(%value : i32) -> u32 [no_inline=yes] {
  slot $cell : obj<4x4>
  block ^entry:
    zeroinit 4x4 $cell
    %p = addr $cell
    store i32 %value, %p
    %read = load u32 %p
    return u32 %read
}

function @main() -> i32 [role=entry] {
  slot $result : obj<8x4>
  block ^entry:
    %value = call obj<8x4> @snapshot()
    copyobj 8x4 %value, $result
    %p = addr $result
    %saved = load i32 %p
    %ok = cmp eq i32 %saved, 7
    branch %ok, ^word, ^failed
  block ^word:
    %bits = call u32 @word(-1)
    %same = cmp eq u32 %bits, 4294967295
    branch %same, ^passed, ^failed
  block ^passed:
    return i32 0
  block ^failed:
    return i32 1
}
