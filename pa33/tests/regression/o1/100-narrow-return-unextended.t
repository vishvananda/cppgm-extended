# A return, a copy to the other sign and a shift's count read the low bits, so
# the mask below needs no extension; the caller extends the result it reads
# wide, even from a callee of this unit.
function @bit(%p : ptr, %n : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %load = load typed u32 %p
    %one = binary shl i32 1, %n
    %mask = copy u32 %one
    %set = binary and u32 %load, %mask
    %out = copy i32 %set
    return i32 %out
}

global @word : u32 = -1

function @main() -> i32 [role=entry] {
  block ^entry:
    %r = call i32 @bit(@word, 31)
    %w = convert sext i64 i32 %r
    %less = cmp lt i64 %w, 0
    branch %less, ^good, ^bad

  block ^good:
    return i32 0

  block ^bad:
    return i32 1
}
