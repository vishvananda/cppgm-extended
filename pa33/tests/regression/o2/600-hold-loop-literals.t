# A loop without calls on its usual paths reads a floating literal from a value
# copied before it, which the allocator keeps in a register; zero and a stored
# literal stay as they are. The call in ^rare runs on a value that is not a
# number, so it does not stop the first loop; the call in the second loop does.
declare function @note(%x : f64) -> f64 [linkage=c, binding=strong, object=note]

function @scale(%p : ptr, %n : i64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: 0, ^next: %step]
    %t = phi f64 [^entry: 0, ^next: %u]
    %x = load typed f64 %p
    %y = binary mul f64 %x, 2.5
    %nan = cmp ne f64 %y, %y
    branch %nan, ^rare, ^usual

  block ^rare:
    %r = call f64 @note(%y)
    jump ^next

  block ^usual:
    store typed f64 0.25, %p
    jump ^next

  block ^next:
    %v = phi f64 [^rare: %r, ^usual: %y]
    %u = binary add f64 %t, %v
    %step = binary add i64 %i, 1
    %c = cmp lt i64 %step, %n
    branch %c, ^head, ^done

  block ^done:
    return f64 %u
}

function @noted(%n : i64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: 0, ^head: %step]
    %r = call f64 @note(1.5)
    %step = binary add i64 %i, 1
    %c = cmp lt i64 %step, %n
    branch %c, ^head, ^done

  block ^done:
    return f64 %r
}
