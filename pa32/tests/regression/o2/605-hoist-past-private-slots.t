# A slot whose address stays among its accesses is reached by no pointer, so a
# write to it leaves a load through a parameter free to leave the loop.
function @private_slot(%p : ptr, %q : ptr, %n : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>

  block ^entry:
    %has = cmp gt i64 %n, 0
    branch %has, ^loop, ^done

  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %next]
    %sum = phi i64 [^entry: 0, ^loop: %more]
    copyobj 16x8 %q, $s
    %a = addr $s
    %b = load typed i64 %a
    %limit = load typed i64 %p
    %c = binary add i64 %b, %limit
    %more = binary add i64 %sum, %c
    %next = binary add nowrap i64 %i, 1
    %again = cmp lt i64 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    %r = phi i64 [^entry: 0, ^loop: %more]
    return i64 %r
}

# Here the slot's address is stored, so %p may name the slot.
function @escaped_slot(%p : ptr, %q : ptr, %n : i64, %out : ptr) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>

  block ^entry:
    %a = addr $s
    store typed ptr %a, %out
    %has = cmp gt i64 %n, 0
    branch %has, ^loop, ^done

  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %next]
    %sum = phi i64 [^entry: 0, ^loop: %more]
    copyobj 16x8 %q, $s
    %limit = load typed i64 %p
    %more = binary add i64 %sum, %limit
    %next = binary add nowrap i64 %i, 1
    %again = cmp lt i64 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    %r = phi i64 [^entry: 0, ^loop: %more]
    return i64 %r
}
