# A branch on a `nowrap` step reads the flags of the step: at 32 bits when only
# its low bits are read, and at 64 bits when something reads it in full, since
# it is canonical there. Neither needs a test.
function @low(%n : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i32 [^entry: %n, ^head: %next]
    %next = binary sub i32 %i, 1
    branch %next, ^head, ^done

  block ^done:
    return i32 %next
}

function @full(%n : i32, %p : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i32 [^entry: %n, ^head: %next]
    %next = binary sub nowrap i32 %i, 1
    %wide = convert sext i64 i32 %next
    store typed i64 %wide, %p
    branch %next, ^head, ^done

  block ^done:
    %r = load typed i64 %p
    return i64 %r
}

global @out : i64 = 7

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @low(3)
    %b = call i64 @full(3, @out)
    %c = convert trunc i32 i64 %b
    %d = binary add i32 %a, %c
    return i32 %d
}
