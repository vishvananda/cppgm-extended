# Check defined values and live ranges under any valid instruction selection.
function @flip(%k : i64, %bits : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %b = binary sub i64 %bits, 1
    %top = binary shl i64 1, %b
    jump ^head
  block ^head:
    %lm = phi i64 [^entry: %top, ^head: %lm2]
    %rm = phi i64 [^entry: 1, ^head: %rm2]
    %r0 = phi i64 [^entry: 0, ^head: %r]
    %hit = binary and i64 %k, %rm
    %set = binary or i64 %r0, %lm
    %r = select i64 %hit, %set, %r0
    %lm2 = binary ushr i64 %lm, 1
    %rm2 = binary shl i64 %rm, 1
    branch %lm2, ^head, ^out
  block ^out:
    %scaled = binary mul i64 %r, 16
    %sum = binary add i64 %scaled, %k
    return i64 %sum
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @flip(6, 4)
    %b = call i64 @flip(1, 3)
    %c = binary add i64 %a, %b
    %w = convert trunc i32 i64 %c
    return i32 %w
}
