# An object slot holds a floating value that a one-byte write also reaches
# (a union of the value and an empty member, as the result of an inlined
# callee that may hold no value has). The eightbyte is one integer field:
# the floating accesses read and write its bits, and the short write clears
# its low byte. The slot splits, so the value stays out of memory and no
# eight-byte read follows a one-byte store.
function @clear_low(%x : f64, %short : i64) -> f64 [binding=strong, no_inline=yes] {
  slot $cell : obj<8x8>

  block ^entry:
    %p = addr $cell
    store f64 %x, %p
    branch %short, ^clear, ^done

  block ^clear:
    zeroinit 1x1 %p
    jump ^done

  block ^done:
    %v = load f64 %p
    return f64 %v
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %step = binary div f64 255, 4503599627370496
    %x = binary add f64 %step, 1
    %cleared = call f64 @clear_low(%x, 1)
    %kept = call f64 @clear_low(%x, 0)
    %one = cmp eq f64 %cleared, 1
    %same = cmp eq f64 %kept, %x
    %a = convert zext i32 i1 %one
    %b = convert zext i32 i1 %same
    %b2 = binary shl i32 %b, 1
    %r = binary or i32 %a, %b2
    return i32 %r
}
