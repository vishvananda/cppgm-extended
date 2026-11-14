# Several readers of one whole-byte field of a loaded word take one load of
# the field. The word's first byte is a kind, and each arm reads it; the
# word itself stays for the bits from 12 up, which are no whole bytes.
global @seen : i64 = 0
global @odd_word : obj<4x4> = { i32 20743 }
global @even_word : obj<4x4> = { i32 8201 }

function @note(%v : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %new = binary add i64 %old, %v
    store i64 %new, @seen
    return void
}

function @visit(%p : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %w = load i32 %p
    %up = binary ushr i32 %w, 12
    %low = binary and i32 %up, 1
    %odd = cmp ne i32 %low, 0
    branch %odd, ^odd, ^even

  block ^odd:
    %kind_here = binary and i32 %w, 255
    %sum = binary add i32 %kind_here, %up
    %wide_sum = convert zext i64 i32 %sum
    call void @note(%wide_sum)
    jump ^done

  block ^even:
    %kind_there = binary and i32 %w, 255
    %product = binary mul i32 %kind_there, 3
    %wide_product = convert zext i64 i32 %product
    call void @note(%wide_product)
    call void @note(1000)
    jump ^done

  block ^done:
    return void
}

# 20743 is kind 7 with 5 above it: 12. 8201 is kind 9 with 2 above it: 27
# and 1000.
function @main() -> i32 {
  block ^entry:
    call void @visit(@odd_word)
    call void @visit(@even_word)
    %total = load i64 @seen
    %ok = cmp eq i64 %total, 1039
    %status = select i32 %ok, 0, 1
    return i32 %status
}
