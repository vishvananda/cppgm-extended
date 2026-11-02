# One of x and x + 1 is even, whatever wraps: halving the product needs no fix-up
# for a negative value. A product of unrelated values keeps its division.
function @triangle(%x : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %y = binary add i32 %x, 1
    %m = binary mul i32 %x, %y
    %h = binary div i32 %m, 2
    return i32 %h
}

function @other(%x : i32, %y : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %m = binary mul i32 %x, %y
    %h = binary div i32 %m, 2
    return i32 %h
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @triangle(-4)
    %b = call i32 @other(-3, 1)
    %c = binary add i32 %a, %b
    return i32 %c
}
