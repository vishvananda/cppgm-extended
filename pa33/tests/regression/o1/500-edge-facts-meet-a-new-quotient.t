# The remainder and the quotient of one division by 10 share one division:
# the simplifier computes the remainder from a new quotient temporary, which
# also replaces the old quotient in the loop test. The edge facts of the
# loop's branch then name that new temporary, so every table the simplifier
# keeps per temporary must cover it. (A bounds-checked build of the optimizer
# read past those tables on this input.)
function @digits(%n : i64, %end : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    jump ^body

  block ^body:
    %v = phi i64 [^entry: %n, ^body: %q]
    %at = phi ptr [^entry: %end, ^body: %before]
    %r = binary umod i64 %v, 10
    %c = binary add i64 %r, 48
    %b = convert trunc i8 i64 %c
    %before = index inbounds i8 %at, -1
    store i8 %b, %before
    %q = binary udiv i64 %v, 10
    %more = cmp ne i64 %q, 0
    branch %more, ^body, ^done

  block ^done:
    return ptr %before
}
