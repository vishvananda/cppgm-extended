# A whole-byte field past a word's first byte is read, and written, through
# that byte alone: its address is an index from the word's, a pointer.
global @word : obj<8x8> = { i64 578437695752307201 }

function @second_byte(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %w = load i64 %p
    %s = binary ushr i64 %w, 8
    %f = binary and i64 %s, 255
    return i64 %f
}

function @set_second_byte(%p : ptr, %v : i32) -> void [no_inline=yes] {
  block ^entry:
    %w = load i32 %p
    %kept = binary and i32 %w, 4294902015
    %b = binary and i32 %v, 255
    %moved = binary shl i32 %b, 8
    %merged = binary or i32 %kept, %moved
    store i32 %merged, %p
    return void
}

function @main() -> i32 {
  block ^entry:
    %before = call i64 @second_byte(@word)
    call void @set_second_byte(@word, 171)
    %after = call i64 @second_byte(@word)
    %whole = load i64 @word
    %before_ok = cmp eq i64 %before, 2
    %after_ok = cmp eq i64 %after, 171
    %whole_ok = cmp eq i64 %whole, 578437695752350465
    %before_bit = select i32 %before_ok, 0, 1
    %after_bit = select i32 %after_ok, 0, 2
    %whole_bit = select i32 %whole_ok, 0, 4
    %some = binary or i32 %before_bit, %after_bit
    %status = binary or i32 %some, %whole_bit
    return i32 %status
}
